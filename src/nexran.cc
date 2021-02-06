
#include "mdclog/mdclog.h"
#include "rmr/RIC_message_types.h"
#include "ricxfcpp/message.hpp"
#include "ricxfcpp/messenger.hpp"

#include "nexran.h"
#include "e2ap.h"
#include "e2sm.h"
#include "e2sm_nexran.h"

namespace nexran {

static void rmr_callback(
    xapp::Message &msg,int mtype,int subid,int payload_len,
    xapp::Msg_component payload,void *data)
{
    ((App *)data)->handle_rmr_message(
        msg,mtype,subid,payload_len,payload);
}

void App::handle_rmr_message(
    xapp::Message &msg,int mtype,int subid,int payload_len,
    xapp::Msg_component &payload)
{
    mdclog_write(MDCLOG_DEBUG,"RMR message (%d)",mtype);
    return;
}

int App::handle_control_ack(e2ap::ControlAck *control)
{

}

int App::handle_control_failure(e2ap::ControlFailure *control)
{

}

int App::handle_indication(e2ap::Indication *indication)
{

}


void App::init()
{
    db[ResourceType::SliceResource][std::string("default")] = new Slice("default");
}

void App::start()
{
    if (running)
	return;

    /*
     * Init the E2.  Note there is nothing to do until we are configured
     * via northbound interface with objects.  Eventually we should
     * store config in the RNIB and restore things on startup, possibly.
     */
    e2ap.init();

    Add_msg_cb(RIC_CONTROL_ACK,rmr_callback,this);
    Add_msg_cb(RIC_CONTROL_FAILURE,rmr_callback,this);
    Add_msg_cb(RIC_INDICATION,rmr_callback,this);

    /*
     * Init and start the northbound interface.
     * NB: the RMR Messenger superclass is already running at this point.
     */
    server.init(this);
    server.start();
    running = true;
}

void App::stop()
{
    /* Stop the northbound interface. */
    server.stop();
    /* Stop the RMR Messenger superclass. */
    Stop();
    running = false;
}

void App::serialize(ResourceType rt,
		    rapidjson::Writer<rapidjson::StringBuffer>& writer)
{
    const char *label;

    mutex.lock();
    writer.StartObject();
    writer.String(rtype_to_label_plural[rt]);
    writer.StartArray();
    for (auto it = db[rt].begin(); it != db[rt].end(); ++it)
	it->second->serialize(writer);
    writer.EndArray();
    writer.EndObject();
    mutex.unlock();
}

bool App::serialize(ResourceType rt,std::string& rname,
		    rapidjson::Writer<rapidjson::StringBuffer>& writer,
		    AppError **ae)
{
    mutex.lock();
    if (db[rt].count(rname) < 1) {
	mutex.unlock();
	if (ae) {
	    if (*ae == NULL)
		*ae = new AppError(404);
	    (*ae)->add(std::string(rtype_to_label[rt])
		       + std::string(" does not exist"));
	}
	return false;
    }

    db[rt][rname]->serialize(writer);
    mutex.unlock();
    return true;
}

bool App::add(ResourceType rt,AbstractResource *resource,
	      rapidjson::Writer<rapidjson::StringBuffer>& writer,
	      AppError **ae)
{
    std::string& rname = resource->getName();

    mutex.lock();
    if (db[rt].count(rname) > 0) {
	if (ae) {
	    if (*ae == NULL)
		*ae = new AppError(403);
	    (*ae)->add(std::string(rtype_to_label[rt])
		       + std::string(" already exists"));
	}
	mutex.unlock();
	return false;
    }

    db[rt][rname] = resource;
    resource->serialize(writer);
    mutex.unlock();
    return true;
}

bool App::del(ResourceType rt,std::string& rname,
	      AppError **ae)
{
    mutex.lock();
    if (db[rt].count(rname) < 1) {
	mutex.unlock();
	if (ae) {
	    if (!*ae)
		*ae = new AppError(404);
	    (*ae)->add(std::string("not found"));
	}
	return false;
    }

    db[rt].erase(rname);
    mutex.unlock();

    return true;
}

bool App::update(ResourceType rt,std::string& rname,
		 rapidjson::Document& d,
		 AppError **ae)
{
    mutex.lock();
    if (db[rt].count(rname) < 1) {
	mutex.unlock();
	if (ae) {
	    if (!*ae)
		*ae = new AppError(404);
	    (*ae)->add(std::string("not found"));
	}
	return false;
    }

    if (!db[rt][rname]->update(d,ae)) {
	mutex.unlock();
	return false;
    }

    mutex.unlock();
    return true;
}

bool App::bind_slice_nodeb(std::string& slice_name,std::string& nodeb_name,
			   AppError **ae)
{
    mutex.lock();
    if (db[App::ResourceType::SliceResource].count(slice_name) < 1) {
	mutex.unlock();
	if (ae) {
	    if (!*ae)
		*ae = new AppError(404);
	    (*ae)->add(std::string("slice does not exist"));
	}
	return false;
    }
    if (db[App::ResourceType::NodeBResource].count(nodeb_name) < 1) {
	mutex.unlock();
	if (ae) {
	    if (!*ae)
		*ae = new AppError(404);
	    (*ae)->add(std::string("nodeb does not exist"));
	}
	return false;
    }
    Slice *slice = (Slice *)db[App::ResourceType::SliceResource][slice_name];
    NodeB *nodeb = (NodeB *)db[App::ResourceType::NodeBResource][nodeb_name];
	
    if (!nodeb->bind_slice(slice)) {
	mutex.unlock();
	if (ae) {
	    if (!*ae)
		*ae = new AppError(403);
	    (*ae)->add(std::string("slice already bound to nodeb"));
	}
	return false;
    }

    ProportionalAllocationPolicy *policy = dynamic_cast<ProportionalAllocationPolicy *>(slice->getPolicy());
    e2sm::nexran::ProportionalAllocationPolicy *npolicy = \
	new e2sm::nexran::ProportionalAllocationPolicy(policy->getShare());
    e2sm::nexran::SliceConfig *sc = new e2sm::nexran::SliceConfig(slice->getName(),npolicy);
    e2sm::nexran::SliceConfigRequest *sreq = new e2sm::nexran::SliceConfigRequest(sc);
    sreq->encode();
    e2ap::ControlRequest *creq = new e2ap::ControlRequest(
        e2ap.get_requestor_id(),e2ap.get_next_instance_id(),
	1,sreq,e2ap::CONTROL_REQUEST_ACK);
    creq->encode();

    // Unlock here; we're done with shared info.
    mutex.unlock();

    std::unique_ptr<xapp::Message> msg = Alloc_msg(creq->get_len());
    msg->Set_mtype(RIC_CONTROL_REQ);
    msg->Set_subid(xapp::Message::NO_SUBID);
    msg->Set_len(creq->get_len());
    xapp::Msg_component payload = msg->Get_payload();
    memcpy((char *)payload.get(),(char *)creq->get_buf(),
	   (msg->Get_available_size() < creq->get_len()) ? msg->Get_available_size() : creq->get_len());
    std::shared_ptr<unsigned char> meid((unsigned char *)strdup(nodeb->getName().c_str()));
    msg->Set_meid(meid);
    msg->Send();

    return true;
}

bool App::unbind_slice_nodeb(std::string& slice_name,std::string& nodeb_name,
			     AppError **ae)
{
    mutex.lock();
    if (db[App::ResourceType::SliceResource].count(slice_name) < 1) {
	mutex.unlock();
	if (ae) {
	    if (!*ae)
		*ae = new AppError(404);
	    (*ae)->add(std::string("slice does not exist"));
	}
	return false;
    }
    if (db[App::ResourceType::NodeBResource].count(nodeb_name) < 1) {
	mutex.unlock();
	if (ae) {
	    if (!*ae)
		*ae = new AppError(404);
	    (*ae)->add(std::string("nodeb does not exist"));
	}
	return false;
    }
    NodeB *nodeb = (NodeB *)db[App::ResourceType::NodeBResource][nodeb_name];
	
    if (!nodeb->unbind_slice(slice_name)) {
	mutex.unlock();
	if (ae) {
	    if (!*ae)
		*ae = new AppError(404);
	    (*ae)->add(std::string("slice not bound to this nodeb"));
	}
	return false;
    }

    e2sm::nexran::SliceDeleteRequest *sreq = \
	new e2sm::nexran::SliceDeleteRequest(slice_name);
    sreq->encode();
    e2ap::ControlRequest *creq = new e2ap::ControlRequest(
        e2ap.get_requestor_id(),e2ap.get_next_instance_id(),
	1,sreq,e2ap::CONTROL_REQUEST_ACK);
    creq->encode();

    // Unlock here; we're done with shared info.
    mutex.unlock();

    std::unique_ptr<xapp::Message> msg = Alloc_msg(creq->get_len());
    msg->Set_mtype(RIC_CONTROL_REQ);
    msg->Set_subid(xapp::Message::NO_SUBID);
    msg->Set_len(creq->get_len());
    xapp::Msg_component payload = msg->Get_payload();
    memcpy((char *)payload.get(),(char *)creq->get_buf(),
	   ((msg->Get_available_size() < creq->get_len())
	    ? msg->Get_available_size() : creq->get_len()));
    std::shared_ptr<unsigned char> meid((unsigned char *)strdup(nodeb->getName().c_str()));
    msg->Set_meid(meid);
    msg->Send();

    return true;
}

bool App::bind_ue_slice(std::string& imsi,std::string& slice_name,
			AppError **ae)
{
    mutex.lock();
    if (db[App::ResourceType::SliceResource].count(slice_name) < 1) {
	mutex.unlock();
	if (ae) {
	    if (!*ae)
		*ae = new AppError(404);
	    (*ae)->add(std::string("slice does not exist"));
	}
	return false;
    }
    if (db[App::ResourceType::UeResource].count(imsi) < 1) {
	mutex.unlock();
	if (ae) {
	    if (!*ae)
		*ae = new AppError(404);
	    (*ae)->add(std::string("ue does not exist"));
	}
	return false;
    }
    Ue *ue = (Ue *)db[App::ResourceType::UeResource][imsi];
    Slice *slice = (Slice *)db[App::ResourceType::SliceResource][slice_name];
	
    if (!slice->bind_ue(ue)) {
	mutex.unlock();
	if (ae) {
	    if (!*ae)
		*ae = new AppError(403);
	    (*ae)->add(std::string("ue already bound to slice"));
	}
	return false;
    }

    e2sm::nexran::SliceUeBindRequest *sreq = \
	new e2sm::nexran::SliceUeBindRequest(slice->getName(),ue->getName());
    sreq->encode();
    // XXX: for all nodebs (later target bound slice/nodeb pairs only.
    for (auto it = db[ResourceType::NodeBResource].begin();
	 it != db[ResourceType::NodeBResource].end();
	 ++it) {
	// Each request needs a different RequestId, so we have to
	// re-encode each time.
	e2ap::ControlRequest *creq = new e2ap::ControlRequest(
            e2ap.get_requestor_id(),e2ap.get_next_instance_id(),
	    1,sreq,e2ap::CONTROL_REQUEST_ACK);
	creq->encode();
	std::unique_ptr<xapp::Message> msg = Alloc_msg(creq->get_len());
	msg->Set_mtype(RIC_CONTROL_REQ);
	msg->Set_subid(xapp::Message::NO_SUBID);
	msg->Set_len(creq->get_len());
	xapp::Msg_component payload = msg->Get_payload();
	memcpy((char *)payload.get(),(char *)creq->get_buf(),
	       ((msg->Get_available_size() < creq->get_len())
		? msg->Get_available_size() : creq->get_len()));
	std::shared_ptr<unsigned char> meid((unsigned char *)strdup(it->second->getName().c_str()));
	msg->Set_meid(meid);
	msg->Send();
    }

    mutex.unlock();
    return true;
}

bool App::unbind_ue_slice(std::string& imsi,std::string& slice_name,
			  AppError **ae)
{
    mutex.lock();
    if (db[App::ResourceType::SliceResource].count(slice_name) < 1) {
	mutex.unlock();
	if (ae) {
	    if (!*ae)
		*ae = new AppError(404);
	    (*ae)->add(std::string("slice does not exist"));
	}
	return false;
    }
    if (db[App::ResourceType::UeResource].count(imsi) < 1) {
	mutex.unlock();
	if (ae) {
	    if (!*ae)
		*ae = new AppError(404);
	    (*ae)->add(std::string("ue does not exist"));
	}
	return false;
    }
    Slice *slice = (Slice *)db[App::ResourceType::SliceResource][slice_name];

    if (!slice->unbind_ue(imsi)) {
	mutex.unlock();
	if (ae) {
	    if (!*ae)
		*ae = new AppError(404);
	    (*ae)->add(std::string("ue not bound to this slice"));
	}
	return false;
    }

    e2sm::nexran::SliceUeUnbindRequest *sreq = \
	new e2sm::nexran::SliceUeUnbindRequest(slice->getName(),imsi);
    sreq->encode();
    // XXX: for all nodebs (later target bound slice/nodeb pairs only.
    for (auto it = db[ResourceType::NodeBResource].begin();
	 it != db[ResourceType::NodeBResource].end();
	 ++it) {
	// Each request needs a different RequestId, so we have to
	// re-encode each time.
	e2ap::ControlRequest *creq = new e2ap::ControlRequest(
            e2ap.get_requestor_id(),e2ap.get_next_instance_id(),
	    1,sreq,e2ap::CONTROL_REQUEST_ACK);
	creq->encode();
	std::unique_ptr<xapp::Message> msg = Alloc_msg(creq->get_len());
	msg->Set_mtype(RIC_CONTROL_REQ);
	msg->Set_subid(xapp::Message::NO_SUBID);
	msg->Set_len(creq->get_len());
	xapp::Msg_component payload = msg->Get_payload();
	memcpy((char *)payload.get(),(char *)creq->get_buf(),
	       ((msg->Get_available_size() < creq->get_len())
		? msg->Get_available_size() : creq->get_len()));
	std::shared_ptr<unsigned char> meid((unsigned char *)strdup(it->second->getName().c_str()));
	msg->Set_meid(meid);
	msg->Send();
    }

    mutex.unlock();
    return true;
}

}

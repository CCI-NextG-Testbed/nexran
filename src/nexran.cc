
#include "nexran.h"

namespace nexran {

void App::init()
{
    db[ResourceType::SliceResource][std::string("default")] = new Slice("default");
}

void App::start()
{
    if (running)
	return;

    server.init(this);
    server.start();
    running = true;
}

void App::stop()
{
    server.stop();
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
    mutex.unlock();
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
    mutex.unlock();
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
    mutex.unlock();
    return true;
}

}

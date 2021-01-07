
//#include <shared>

/*

/version -- 200, { "version": "<version-string>" }
/slice(<id>?)
* GET: -- no args, list of slices; GET <id> single slice
{ "id":<id>,[mix of kpm metrics and xran metrics, and cell config] }
* PUT: -- slice config
/ue
* PUT: -- UE slice binding; 
/metrics/slice/<slice-id> (probably static indexing of 
/metrics/ran/<ran-id>
(probably streaming versions of metrics that update with KPM reports eventually)


 */


#include "nexran.h"
#include "version.h"
#include "restserver.h"

#define HANDLE_APP_ERROR(ae,default_code)				\
    do {								\
        if (ae) {							\
	    ae->serialize(writer);				\
	    response.send((Pistache::Http::Code)ae->http_status,	\
			  sb.GetString());				\
	}								\
	else								\
	    response.send(default_code);				\
    } while (0);

namespace nexran {

#define VERSION_PREFIX "/v1/"

void RestServer::setupRoutes()
{
    Pistache::Rest::Routes::Get(
	router,VERSION_PREFIX "/version",
	Pistache::Rest::Routes::bind(&RestServer::getVersion,this));

    Pistache::Rest::Routes::Get(
	router,VERSION_PREFIX "/nodebs/:name",
	Pistache::Rest::Routes::bind(&RestServer::getNodeB,this));
    Pistache::Rest::Routes::Get(
	router,VERSION_PREFIX "/nodebs",
	Pistache::Rest::Routes::bind(&RestServer::getNodeBs,this));
    Pistache::Rest::Routes::Post(
	router,VERSION_PREFIX "/nodebs",
	Pistache::Rest::Routes::bind(&RestServer::postNodeB,this));
    Pistache::Rest::Routes::Put(
	router,VERSION_PREFIX "/nodebs/:name",
	Pistache::Rest::Routes::bind(&RestServer::putNodeB,this));
    Pistache::Rest::Routes::Delete(
	router,VERSION_PREFIX "/nodebs/:name",
	Pistache::Rest::Routes::bind(&RestServer::deleteNodeB,this));

    Pistache::Rest::Routes::Post(
	router,VERSION_PREFIX "/nodebs/:nodeb_name/slices/:slice_name",
	Pistache::Rest::Routes::bind(&RestServer::postNodeBSliceBinding,this));
    Pistache::Rest::Routes::Delete(
	router,VERSION_PREFIX "/nodebs/:nodeb_name/slices/:slice_name",
	Pistache::Rest::Routes::bind(&RestServer::deleteNodeBSliceBinding,this));

    Pistache::Rest::Routes::Get(
	router,VERSION_PREFIX "/slices/:name",
	Pistache::Rest::Routes::bind(&RestServer::getSlice,this));
    Pistache::Rest::Routes::Get(
	router,VERSION_PREFIX "/slices",
	Pistache::Rest::Routes::bind(&RestServer::getSlices,this));
    Pistache::Rest::Routes::Post(
	router,VERSION_PREFIX "/slices",
	Pistache::Rest::Routes::bind(&RestServer::postSlice,this));
    Pistache::Rest::Routes::Put(
	router,VERSION_PREFIX "/slices/:name",
	Pistache::Rest::Routes::bind(&RestServer::putSlice,this));
    Pistache::Rest::Routes::Delete(
	router,VERSION_PREFIX "/slices/:name",
	Pistache::Rest::Routes::bind(&RestServer::deleteSlice,this));

    Pistache::Rest::Routes::Post(
	router,VERSION_PREFIX "/slices/:slice_name/ues/:imsi",
	Pistache::Rest::Routes::bind(&RestServer::postSliceUeBinding,this));
    Pistache::Rest::Routes::Delete(
	router,VERSION_PREFIX "/slices/:slice_name/ues/:imsi",
	Pistache::Rest::Routes::bind(&RestServer::deleteSliceUeBinding,this));

    Pistache::Rest::Routes::Get(
	router,VERSION_PREFIX "/ues/:imsi",
	Pistache::Rest::Routes::bind(&RestServer::getUe,this));
    Pistache::Rest::Routes::Get(
	router,VERSION_PREFIX "/ues",
	Pistache::Rest::Routes::bind(&RestServer::getUes,this));
    Pistache::Rest::Routes::Post(
	router,VERSION_PREFIX "/ues",
	Pistache::Rest::Routes::bind(&RestServer::postUe,this));
    Pistache::Rest::Routes::Put(
	router,VERSION_PREFIX "/ues/:imsi",
	Pistache::Rest::Routes::bind(&RestServer::putUe,this));
    Pistache::Rest::Routes::Delete(
	router,VERSION_PREFIX "/ues/:imsi",
	Pistache::Rest::Routes::bind(&RestServer::deleteUe,this));
}

void RestServer::init(App *app_)
{
    Pistache::Address addr("*:8000");

    app = app_;

    setupRoutes();
    endpoint.init();
    endpoint.setHandler(router.handler());
    endpoint.bind(addr);
    //Pistache::Address(Pistache::Ipv4::any(),Pistache::Port(8181)));
    //Pistache::Ipv4::any(),Pistache::Port(8181)))) {};
}

void RestServer::start()
{
    if (running)
	return;

    endpoint.serveThreaded();
    running = true;
}

std::string RestServer::buildVersionJson()
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    writer.StartObject();
    writer.String("version");
    writer.String(NEXRAN_VERSION_STRING);
    writer.String("major");
    writer.Uint(NEXRAN_VERSION_MAJOR);
    writer.String("minor");
    writer.Uint(NEXRAN_VERSION_MINOR);
    writer.String("patch");
    writer.Uint(NEXRAN_VERSION_PATCH);
    writer.String("commit");
    writer.String(NEXRAN_GIT_COMMIT);
    writer.String("tag");
    writer.String(NEXRAN_GIT_TAG);
    writer.String("branch");
    writer.String(NEXRAN_GIT_BRANCH);
    writer.String("buildTimestamp");
    writer.String(NEXRAN_BUILD_TIMESTAMP);
    writer.EndObject();

    return sb.GetString();
}

void RestServer::getVersion(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    response.send(
        Pistache::Http::Code::Ok,versionJson.c_str());
}

void RestServer::getNodeBs(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    app->serialize(App::ResourceType::NodeBResource,writer);
    response.send(Pistache::Http::Code::Ok,sb.GetString());
}

void RestServer::postNodeB(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    AppError *ae = NULL;
    rapidjson::Document d;

    d.Parse(request.body().c_str());

    NodeB *nb = NodeB::create(d,&ae);
    if (!nb) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Bad_Request);
	return;
    }
    if (!app->add(App::ResourceType::NodeBResource,nb,writer,&ae)) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Bad_Request);
	return;
    }

    response.send(Pistache::Http::Code::Created,sb.GetString());
}

void RestServer::postNodeBSliceBinding(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    AppError *ae = NULL;
    auto nodeb_name = request.param(":nodeb_name").as<std::string>();
    auto slice_name = request.param(":slice_name").as<std::string>();

    if (!app->bind_slice_nodeb(slice_name,nodeb_name,&ae)) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Bad_Request);
	return;
    }

    response.send(Pistache::Http::Code::Ok);
}

void RestServer::deleteNodeBSliceBinding(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    AppError *ae = NULL;
    auto nodeb_name = request.param(":nodeb_name").as<std::string>();
    auto slice_name = request.param(":slice_name").as<std::string>();

    if (!app->unbind_slice_nodeb(slice_name,nodeb_name,&ae)) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Bad_Request);
	return;
    }

    response.send(Pistache::Http::Code::Ok);
}

void RestServer::postSliceUeBinding(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    AppError *ae = NULL;
    auto slice_name = request.param(":slice_name").as<std::string>();
    auto imsi = request.param(":imsi").as<std::string>();

    if (!app->bind_ue_slice(imsi,slice_name,&ae)) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Bad_Request);
	return;
    }

    response.send(Pistache::Http::Code::Ok);
}

void RestServer::deleteSliceUeBinding(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    AppError *ae = NULL;
    auto slice_name = request.param(":slice_name").as<std::string>();
    auto imsi = request.param(":imsi").as<std::string>();

    if (!app->unbind_ue_slice(imsi,slice_name,&ae)) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Bad_Request);
	return;
    }

    response.send(Pistache::Http::Code::Ok);
}

void RestServer::putNodeB(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    AppError *ae = NULL;
    auto name = request.param(":name").as<std::string>();
    rapidjson::Document d;

    d.Parse(request.body().c_str());

    if (!app->update(App::ResourceType::NodeBResource,name,d,&ae)) {
	if (ae) {
	    ae->serialize(writer);
	    response.send((Pistache::Http::Code)ae->http_status,
			  sb.GetString());
	}
	else
	    response.send(Pistache::Http::Code::Bad_Request);
	return;
    }

    response.send(Pistache::Http::Code::Ok);
}

void RestServer::deleteNodeB(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    auto name = request.param(":name").as<std::string>();
    AppError *ae = NULL;

    if (!app->del(App::ResourceType::NodeBResource,name,&ae)) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Internal_Server_Error);
	return;
    }

    response.send(Pistache::Http::Code::Ok);
}

void RestServer::getNodeB(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    auto name = request.param(":name").as<std::string>();
    AppError *ae = NULL;

    if (!app->serialize(App::ResourceType::NodeBResource,name,writer,&ae)) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Internal_Server_Error);
	return;
    }

    response.send(Pistache::Http::Code::Ok,sb.GetString());
}

void RestServer::getSlices(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    app->serialize(App::ResourceType::SliceResource,writer);
    response.send(Pistache::Http::Code::Ok,sb.GetString());
}

void RestServer::postSlice(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    AppError *ae = NULL;
    rapidjson::Document d;

    d.Parse(request.body().c_str());

    Slice *slice = Slice::create(d,&ae);
    if (!slice) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Bad_Request);
	return;
    }
    if (!app->add(App::ResourceType::SliceResource,slice,writer,&ae)) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Bad_Request);
	return;
    }

    response.send(Pistache::Http::Code::Created,sb.GetString());
}

void RestServer::putSlice(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    AppError *ae = NULL;
    auto name = request.param(":name").as<std::string>();
    rapidjson::Document d;

    d.Parse(request.body().c_str());

    if (!app->update(App::ResourceType::SliceResource,name,d,&ae)) {
	if (ae) {
	    ae->serialize(writer);
	    response.send((Pistache::Http::Code)ae->http_status,
			  sb.GetString());
	}
	else
	    response.send(Pistache::Http::Code::Bad_Request);
	return;
    }

    response.send(Pistache::Http::Code::Ok);
}

void RestServer::getSlice(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    AppError *ae = NULL;
    auto name = request.param(":name").as<std::string>();

    if (!app->serialize(App::ResourceType::SliceResource,name,writer,&ae)) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Internal_Server_Error);
	return;
    }

    response.send(Pistache::Http::Code::Ok,sb.GetString());
}

void RestServer::deleteSlice(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    auto name = request.param(":name").as<std::string>();
    AppError *ae = NULL;

    if (!app->del(App::ResourceType::SliceResource,name,&ae)) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Internal_Server_Error);
	return;
    }

    response.send(Pistache::Http::Code::Ok);
}

void RestServer::getUes(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    app->serialize(App::ResourceType::UeResource,writer);
    response.send(Pistache::Http::Code::Ok,sb.GetString());
}

void RestServer::postUe(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    AppError *ae = NULL;
    rapidjson::Document d;

    d.Parse(request.body().c_str());

    Ue *ue = Ue::create(d,&ae);
    if (!ue) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Bad_Request);
	return;
    }
    if (!app->add(App::ResourceType::UeResource,ue,writer,&ae)) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Bad_Request);
	return;
    }

    response.send(Pistache::Http::Code::Created,sb.GetString());
}

void RestServer::putUe(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    AppError *ae = NULL;
    auto imsi = request.param(":imsi").as<std::string>();
    rapidjson::Document d;

    d.Parse(request.body().c_str());

    if (!app->update(App::ResourceType::UeResource,imsi,d,&ae)) {
	if (ae) {
	    ae->serialize(writer);
	    response.send((Pistache::Http::Code)ae->http_status,
			  sb.GetString());
	}
	else
	    response.send(Pistache::Http::Code::Bad_Request);
	return;
    }

    response.send(Pistache::Http::Code::Ok);
}

void RestServer::getUe(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    AppError *ae = NULL;
    auto imsi = request.param(":imsi").as<std::string>();

    if (!app->serialize(App::ResourceType::UeResource,imsi,writer,&ae)) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Internal_Server_Error);
	return;
    }

    response.send(Pistache::Http::Code::Ok,sb.GetString());
}

void RestServer::deleteUe(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter response)
{
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    auto imsi = request.param(":imsi").as<std::string>();
    AppError *ae = NULL;

    if (!app->del(App::ResourceType::UeResource,imsi,&ae)) {
	HANDLE_APP_ERROR(ae,Pistache::Http::Code::Internal_Server_Error);
	return;
    }

    response.send(Pistache::Http::Code::Ok);
}

void RestServer::stop()
{
    if (!running)
	return;

    endpoint.shutdown();
    running = false;
}

}

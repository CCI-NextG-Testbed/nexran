#ifndef __RESTSERVER__H__
#define __RESTSERVER__H__

#include <string>

#include <pistache/endpoint.h>
#include <pistache/router.h>

namespace nexran {

class App;

class RestServer {
 public:

    RestServer()
	: running(false), app(NULL), versionJson(buildVersionJson()) {};
    virtual void init(App *app_);
    virtual void start();
    virtual void stop();
    virtual ~RestServer() { stop(); };
    static std::string buildVersionJson();

 private:
    void setupRoutes();

    void getVersion(const Pistache::Rest::Request &request,
		    Pistache::Http::ResponseWriter response);

    void getNodeBs(const Pistache::Rest::Request &request,
		   Pistache::Http::ResponseWriter response);
    void postNodeB(const Pistache::Rest::Request &request,
		   Pistache::Http::ResponseWriter response);
    void putNodeB(const Pistache::Rest::Request &request,
		  Pistache::Http::ResponseWriter response);
    void getNodeB(const Pistache::Rest::Request &request,
		  Pistache::Http::ResponseWriter response);
    void deleteNodeB(const Pistache::Rest::Request &request,
		     Pistache::Http::ResponseWriter response);

    void getSlices(const Pistache::Rest::Request &request,
		   Pistache::Http::ResponseWriter response);
    void postSlice(const Pistache::Rest::Request &request,
		   Pistache::Http::ResponseWriter response);
    void putSlice(const Pistache::Rest::Request &request,
		  Pistache::Http::ResponseWriter response);
    void getSlice(const Pistache::Rest::Request &request,
		  Pistache::Http::ResponseWriter response);
    void deleteSlice(const Pistache::Rest::Request &request,
		     Pistache::Http::ResponseWriter response);

    void getUes(const Pistache::Rest::Request &request,
		Pistache::Http::ResponseWriter response);
    void postUe(const Pistache::Rest::Request &request,
		Pistache::Http::ResponseWriter response);
    void putUe(const Pistache::Rest::Request &request,
	       Pistache::Http::ResponseWriter response);
    void getUe(const Pistache::Rest::Request &request,
	       Pistache::Http::ResponseWriter response);
    void deleteUe(const Pistache::Rest::Request &request,
		  Pistache::Http::ResponseWriter response);

    void postNodeBSliceBinding(const Pistache::Rest::Request &request,
			       Pistache::Http::ResponseWriter response);
    void deleteNodeBSliceBinding(const Pistache::Rest::Request &request,
				 Pistache::Http::ResponseWriter response);

    void postSliceUeBinding(const Pistache::Rest::Request &request,
			    Pistache::Http::ResponseWriter response);
    void deleteSliceUeBinding(const Pistache::Rest::Request &request,
			      Pistache::Http::ResponseWriter response);

    bool running;
    App *app;
    Pistache::Http::Endpoint endpoint;
    Pistache::Rest::Router router;
    const std::string versionJson;
};

}

#endif /* __RESTSERVER__H__ */

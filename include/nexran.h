#ifndef _NEXRAN_H_
#define _NEXRAN_H_

#include <string>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <cstring>
#include <cstdint>
#include <cstdio>

#include "rapidjson/prettywriter.h"
#include "rapidjson/document.h"

#include "restserver.h"

namespace nexran {

class AppError {
 public:
    AppError(int http_status_,std::list<std::string> messages_)
	: http_status(http_status_),messages(messages_) {};
    AppError(int http_status_,std::string message_)
	: http_status(http_status_),
	  messages(std::list<std::string>({message_})) {};
    AppError(int http_status_)
	: http_status(http_status_),
	  messages(std::list<std::string>({})) {};
    virtual ~AppError() = default;

    void add(std::string message)
    {
	messages.push_back(message);
    };

    void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer)
    {
	writer.StartObject();
	writer.String("errors");
	writer.StartArray();
	for (auto it = messages.begin(); it != messages.end(); ++it) {
	    writer.String(it->c_str());
	}
	writer.EndArray();
	writer.EndObject();
    };

    int status;
    int http_status;
    std::list<std::string> messages;
};

class AbstractResource {
 public:
    virtual ~AbstractResource() = default;
    virtual std::string& getName() = 0;
    virtual void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer) {};
    virtual bool update(rapidjson::Document& d,
			AppError **ae) { return false; };
};

typedef enum {
    STRING = 0,
    INT,
    UINT,
    FLOAT,
    OBJECT,
    ARRAY,
    BOOL
} JsonTypeMap;

typedef enum {
    GET = 0,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH,
} HttpMethod;

template <class T>
class Resource : public AbstractResource {
 public:

    static bool validate_value(std::string& k,const rapidjson::Value& v)
    {
	if (T::propertyTypes.count(k) < 1)
	    return false;
	switch (T::propertyTypes[k]) {
	case JsonTypeMap::STRING: return v.IsString();
	case JsonTypeMap::INT:    return v.IsInt();
	case JsonTypeMap::UINT:   return v.IsUint();
	case JsonTypeMap::FLOAT:  return v.IsFloat();
	case JsonTypeMap::OBJECT: return v.IsObject();
	case JsonTypeMap::ARRAY:  return v.IsArray();
	case JsonTypeMap::BOOL:   return v.IsBool();
	default:                  return false;
	}
    };

    static bool validate_json
	(HttpMethod m,const rapidjson::Value& obj,AppError **ae)
    {
	bool retval = true;
	if (T::required.count(m) == 1) {
	    for (auto it = T::required[m].begin(); it != T::required[m].end(); ++it) {
		if (!obj.HasMember(it->c_str())) {
		    if (ae) {
			if (!*ae)
			    *ae = new AppError(400);
			(*ae)->add(std::string("missing required property: ") + *it);
		    }
		    if (T::propertyErrorImmediateAbort)
			return false;
		    else
			retval = false;
		}
		else if (!validate_value(*it,obj[it->c_str()])) {
		    if (ae) {
			if (!*ae)
			    *ae = new AppError(400);
			(*ae)->add(std::string("invalid type for property: ") + *it);
		    }
		    if (T::propertyErrorImmediateAbort)
			return false;
		    else
			retval = false;
		}
	    }
	}
	if (T::optional.count(m) == 1) {
	    for (auto it = T::optional[m].begin(); it != T::optional[m].end(); ++it) {
		if (!obj.HasMember(it->c_str()))
		    continue;
		else if (!validate_value(*it,obj[it->c_str()])) {
		    if (ae) {
			if (!*ae)
			    *ae = new AppError(400);
			(*ae)->add(std::string("invalid type for property: ") + *it);
		    }
		    if (T::propertyErrorImmediateAbort)
			return false;
		    else
			retval = false;
		}
	    }
	}
	if (T::disallowed.count(m) == 1) {
	    for (auto it = T::disallowed[m].begin(); it != T::disallowed[m].end(); ++it) {
		if (!obj.HasMember(it->c_str()))
		    continue;
		if (ae) {
		    if (!*ae)
			*ae = new AppError(400);
		    (*ae)->add(std::string("invalid property: ") + *it);
		}
		if (T::propertyErrorImmediateAbort)
		    return false;
		else
		    retval = false;
	    }
	}
	for (auto it = T::propertyEnums.begin(); it != T::propertyEnums.end(); ++it) {
	    if (obj.HasMember(it->first.c_str())
		&& obj[it->first.c_str()].IsString()
		&& std::find(it->second.begin(),it->second.end(),
			     std::string(obj[it->first.c_str()].GetString()))
		    == it->second.end()) {
	    if (ae) {
		if (!*ae)
		    *ae = new AppError(400);
		(*ae)->add(std::string("invalid enum value for property: ") + it->first);
	    }
	    if (T::propertyErrorImmediateAbort)
		return false;
	    else
		retval = false;
	    }
	}
	return retval;
    };
};

class Ue : public Resource<Ue> {
 public:
    static std::map<std::string,JsonTypeMap> propertyTypes;
    static std::map<std::string,std::list<std::string>> propertyEnums;
    static std::map<HttpMethod,std::list<std::string>> required;
    static std::map<HttpMethod,std::list<std::string>> optional;
    static std::map<HttpMethod,std::list<std::string>> disallowed;
    static const bool propertyErrorImmediateAbort = false;

    Ue(const std::string& imsi_)
	: imsi(imsi_) {};
    Ue(const std::string& imsi_,const std::string& tmsi_,
       const std::string& crnti_)
	: imsi(imsi_),tmsi(tmsi_),crnti(crnti_) {};
    virtual ~Ue() = default;

    std::string& getName() { return imsi; }
    void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer);
    static Ue *create(rapidjson::Document& d,AppError **ae);
    bool update(rapidjson::Document& d,AppError **ae);

 private:
    std::string imsi;
    std::string tmsi;
    std::string crnti;
    bool connected;
};

class AllocationPolicy {
 public:
    typedef enum {
	Proportional = 1,
    } Type;

    static std::map<Type,const char *> type_to_string;

    virtual ~AllocationPolicy() = default;
    virtual const char *getName() = 0;
    virtual const Type getType() = 0;
    virtual void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer) = 0;
    virtual bool update(const rapidjson::Value& obj,AppError **ae) = 0;
};

class ProportionalAllocationPolicy : public AllocationPolicy {
 public:
    ProportionalAllocationPolicy(int share_) : share(share_) {};
    ~ProportionalAllocationPolicy() = default;

    const char *getName() { return name; }
    const AllocationPolicy::Type getType() { return AllocationPolicy::Type::Proportional; }
    void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer)
    {
	writer.StartObject();
	writer.String("type");
	writer.String("proportional");
	writer.String("share");
	writer.Int(share);
	writer.EndObject();
    };
    bool update(const rapidjson::Value& obj,AppError **ae);

 private:
    static constexpr const char *name = "proportional";

    int share;
};

class Slice : public Resource<Slice> {
 public:
    static std::map<std::string,JsonTypeMap> propertyTypes;
    static std::map<std::string,std::list<std::string>> propertyEnums;
    static std::map<HttpMethod,std::list<std::string>> required;
    static std::map<HttpMethod,std::list<std::string>> optional;
    static std::map<HttpMethod,std::list<std::string>> disallowed;
    static const bool propertyErrorImmediateAbort = false;

    Slice(const std::string& name_)
	: name(name_),
	  allocation_policy(new ProportionalAllocationPolicy(1024)) {};
    Slice(const std::string& name_,AllocationPolicy *allocation_policy_)
	: name(name_),
	  allocation_policy(allocation_policy_) {};
    virtual ~Slice() = default;

    std::string& getName() { return name; }
    void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer);
    static Slice *create(rapidjson::Document& d,AppError **ae);
    bool update(rapidjson::Document& d,AppError **ae);
    bool bind_ue(Ue *ue) {
	if (ues.count(ue->getName()) > 0)
	    return false;
	ues[std::string(ue->getName())] = ue;
	return true;
    };
    bool unbind_ue(std::string& imsi) {
	if (ues.count(imsi) < 1)
	    return false;
	ues.erase(imsi);
	return true;
    };

 private:
    std::string name;
    AllocationPolicy *allocation_policy;
    std::map<std::string, Ue *> ues;
};

class NodeB : public Resource<NodeB> {
 public:
    static std::map<std::string,JsonTypeMap> propertyTypes;
    static std::map<std::string,std::list<std::string>> propertyEnums;
    static std::map<HttpMethod,std::list<std::string>> required;
    static std::map<HttpMethod,std::list<std::string>> optional;
    static std::map<HttpMethod,std::list<std::string>> disallowed;
    static const bool propertyErrorImmediateAbort = false;

    typedef enum {
	UNKNOWN = 0,
	GNB = 1,
	GNB_CU_UP,
	GNB_DU,
	EN_GNB,
	ENB,
	NG_ENB,
	__END__
    } Type;

    static const char *type_to_type_string(Type t);
    static Type type_string_to_type(const char *ts);
    static const char *get_name_prefix(Type type,uint8_t id_len);

    static std::unique_ptr<std::string>	build_name(
	Type type,const char *mcc,const char *mnc,
	int32_t id,uint8_t id_len);

    NodeB(Type type_,const char *mcc_,const char *mnc_,
	  int32_t id_,uint8_t id_len_)
	: type(type_),mcc(std::string(mcc_)),mnc(std::string(mnc_)),
	  id(id_),id_len(id_len_),connected(false),total_prbs(-1),
	  name(build_name(type_,mcc_,mnc_,id_,id_len_)) {};
    virtual ~NodeB() = default;

    std::string& getName() { return *name; }
    void serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer);
    static NodeB *create(rapidjson::Document& d,AppError **ae);
    bool update(rapidjson::Document& d,AppError **ae);
    bool bind_slice(Slice *slice) {
	if (slices.count(slice->getName()) > 0)
	    return false;
	slices[std::string(slice->getName())] = slice;
	return true;
    };
    bool unbind_slice(std::string& slice_name) {
	if (slices.count(slice_name) < 1)
	    return false;
	slices.erase(slice_name);
	return true;
    };

 private:
    static const char *type_string_map[NodeB::Type::__END__];

    std::unique_ptr<std::string> name;
    Type type;
    std::string mcc;
    std::string mnc;
    int32_t id;
    uint8_t id_len;
    bool connected;
    int total_prbs;
    std::map<std::string,Slice *> slices;
};

class SliceMetrics {
 public:
    SliceMetrics(const Slice& slice_)
	: slice(slice_) {};
    virtual ~SliceMetrics() {};

 private:
    const Slice& slice;
};

class Config {
 public:
    /*
    typedef struct ConfigItem {

    } ConfigItem_t;

    typedef enum {
    */

    Config() {};
    virtual ~Config() {};
    virtual bool load() { return true; };

 private:
    bool loadArgv(int argc,const char **argv) { return false; };
    bool loadEnv() { return false; };

    std::map<std::string *,std::string *> config;
};

class App {
 public:
    typedef enum {
	NodeBResource = 0,
	SliceResource,
	UeResource,
    } ResourceType;

    App(Config &config_) : config(config_), running(false) { };
    virtual ~App() = default;
    virtual void init();
    virtual void start();
    virtual void stop();

    void serialize(ResourceType rt,
		   rapidjson::Writer<rapidjson::StringBuffer>& writer);
    bool serialize(ResourceType rt,std::string& rname,
		   rapidjson::Writer<rapidjson::StringBuffer>& writer,
		   AppError **ae);
    bool add(ResourceType rt,AbstractResource *resource,
	     rapidjson::Writer<rapidjson::StringBuffer>& writer,
	     AppError **ae);
    bool del(ResourceType rt,std::string& rname,
	     AppError **ae);
    bool update(ResourceType rt,std::string& rname,
		rapidjson::Document& d,AppError **ae);

    bool bind_slice_nodeb(std::string& slice_name,std::string& nodeb_name,
			  AppError **ae);
    bool unbind_slice_nodeb(std::string& slice_name,std::string& nodeb_name,
			    AppError **ae);
    bool bind_ue_slice(std::string& imsi,std::string& slice_name,
		       AppError **ae);
    bool unbind_ue_slice(std::string& imsi,std::string& slice_name,
			 AppError **ae);

 private:
    Config &config;
    bool running;
    RestServer server;
    std::mutex mutex;
    std::map<ResourceType,std::map<std::string,AbstractResource *>> db;
    std::map<ResourceType,const char *> rtype_to_label = {
	{ ResourceType::NodeBResource, "nodeb" },
	{ ResourceType::SliceResource, "slice" },
	{ ResourceType::UeResource, "ue" }
    };
    std::map<ResourceType,const char *> rtype_to_label_plural = {
	{ ResourceType::NodeBResource, "nodebs" },
	{ ResourceType::SliceResource, "slices" },
	{ ResourceType::UeResource, "ues" }
    };
};

}

#endif /* _NEXRAN_H_ */

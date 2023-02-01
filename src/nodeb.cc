
#include "nexran.h"

namespace nexran {

std::map<std::string,JsonTypeMap> NodeB::propertyTypes = {
    { "name",JsonTypeMap::STRING },
    { "type",JsonTypeMap::STRING },
    { "mcc",JsonTypeMap::STRING },
    { "mnc",JsonTypeMap::STRING },
    { "id",JsonTypeMap::INT },
    { "id_len",JsonTypeMap::UINT },
    { "status",JsonTypeMap::OBJECT },
    { "config",JsonTypeMap::OBJECT },
    { "dl_rbg_mask",JsonTypeMap::STRING },
    { "ul_prb_mask",JsonTypeMap::STRING }
};
std::map<std::string,std::list<std::string>> NodeB::propertyEnums = {
    { "type",{ "gNB","gNB-CU-UP","gNB-DU","en-gNB","eNB","ng-eNB" } },
};
std::map<HttpMethod,std::list<std::string>> NodeB::required = {
    { HttpMethod::POST,{ "type","id","mcc","mnc" } }
};
std::map<HttpMethod,std::list<std::string>> NodeB::optional = {
    { HttpMethod::POST,{ "id_len","dl_rbg_mask","ul_prb_mask" } },
    { HttpMethod::PUT,{ "dl_rbg_mask","ul_prb_mask" } }
};
std::map<HttpMethod,std::list<std::string>> NodeB::disallowed = {
    { HttpMethod::POST,{ "name","status","config" } },
    { HttpMethod::PUT,{ "type","id","id_len","mcc","mnc","name","status","config" } }
};

const char *NodeB::type_string_map[NodeB::Type::__END__] = {
    "unknown","gNB","gNB-CU-UP","gNB-DU","en-gNB","eNB","ng-eNB"
};

const char *NodeB::type_to_type_string(Type t)
{
    if (t >= Type::UNKNOWN && t < Type::__END__)
	return type_string_map[(int)t];
    return "invalid";
}

NodeB::Type NodeB::type_string_to_type(const char *ts)
{
    int i;
    for (i = 0; i < (int)Type::__END__; ++i)
	if (strcmp(ts,type_string_map[i]) == 0)
	    break;
    return (Type)i;
}

const char *NodeB::get_name_prefix(Type type,uint8_t id_len)
{
    if (type == Type::UNKNOWN)
	return "unknown";
    else if (type >= Type::GNB && type <= GNB_DU)
	return "gnB";
    else if (type == Type::EN_GNB)
	return "en_gnB";
    else if (type == Type::ENB)
	if (id_len == 0 || id_len == 20)
	    return "enB_macro";
	else if (id_len == 18)
	    return "enB_shortmacro";
	else if (id_len == 21)
	    return "enB_longmacro";
	else
	    return "invalid";
    else if (type == Type::NG_ENB)
	if (id_len == 0 || id_len == 20)
	    return "ng_enB_macro";
	else if (id_len == 18)
	    return "ng_enB_shortmacro";
	else if (id_len == 21)
	    return "ng_enB_longmacro";
	else
	    return "invalid";
    else
	return "invalid";
}

std::unique_ptr<std::string> NodeB::build_name(
    Type type,const char *mcc,const char *mnc,
    int32_t id,uint8_t id_len)
{
    char buf[128];
    int rc = 0;
    const char *name_prefix =
	get_name_prefix(type,id_len);
    const char *mnc_prefix = "";
    if (strlen(mnc) == 1)
	mnc_prefix = "00";
    else if (strlen(mnc) == 2)
	mnc_prefix = "0";

    rc = std::snprintf(buf,sizeof(buf),"%s_%s_%s%s_%05x0",
		       name_prefix,mcc,mnc_prefix,mnc,id);
    buf[rc] = '\0';
    return std::make_unique<std::string>(buf);
}

void NodeB::serialize(rapidjson::Writer<rapidjson::StringBuffer>& writer)
{
    writer.StartObject();
    writer.String("name");
    writer.String(name->c_str());
    writer.String("type");
    writer.String(type_to_type_string(type));
    writer.String("id");
    writer.Int(id);
    writer.String("id_len");
    writer.Uint(id_len);
    writer.String("status");
    writer.StartObject();
    writer.String("connected");
    writer.Bool(connected);
    writer.EndObject();
    writer.String("config");
    writer.StartObject();
    writer.String("total_prbs");
    writer.Int(total_prbs);
    writer.EndObject();
    writer.String("slices");
    writer.StartArray();
    for (auto it = slices.begin(); it != slices.end(); ++it) {
	writer.String(it->first.c_str());
    }
    writer.EndArray();
    writer.String("dl_rbg_mask");
    writer.String(dl_rbg_mask.c_str());
    writer.String("ul_prb_mask");
    writer.String(ul_prb_mask.c_str());
    writer.EndObject();
};

NodeB *NodeB::create(rapidjson::Document& d,AppError **ae)
{
    if (!d.IsObject()) {
	if (ae) {
	    if (!*ae)
		*ae = new AppError(400);
	    (*ae)->add(std::string("request is not an object"));
	}
	return NULL;
    }
    const rapidjson::Value& obj = d.GetObject();
    if (!NodeB::validate_json(HttpMethod::POST,obj,ae))
	return NULL;

    uint8_t id_len = 20;
    if (obj.HasMember("id_len"))
	id_len = (uint8_t)obj["id_len"].GetUint();
    NodeB *nb = new NodeB(
	type_string_to_type(obj["type"].GetString()),
	obj["mcc"].GetString(),obj["mnc"].GetString(),
	obj["id"].GetInt(),id_len);
    if (obj.HasMember("dl_rbg_mask"))
	nb->set_dl_rbg_mask(obj["dl_rbg_mask"].GetString());
    if (obj.HasMember("ul_prb_mask"))
	nb->set_ul_prb_mask(obj["ul_prb_mask"].GetString());
    return nb;
}

bool NodeB::update(rapidjson::Document& d,AppError **ae)
{
    if (!d.IsObject()) {
	if (ae) {
	    if (!*ae)
		*ae = new AppError(400);
	    (*ae)->add(std::string("request is not an object"));
	}
	return NULL;
    }

    const rapidjson::Value& obj = d.GetObject();
    if (!NodeB::validate_json(HttpMethod::PUT,obj,ae))
	return false;

    if (obj.HasMember("dl_rbg_mask"))
	set_dl_rbg_mask(obj["dl_rbg_mask"].GetString());
    if (obj.HasMember("ul_prb_mask"))
	set_ul_prb_mask(obj["ul_prb_mask"].GetString());

    return true;
}

}

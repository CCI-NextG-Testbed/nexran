
#include <cstdlib>
#include <cstdio>
#include <iostream>

#include "config.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace rapidjson;

nexran::Config::Config()
{
    config[XAPP_NAME] = new Item(
	STRING,'n',"xapp-name","XAPP_NAME",true,(ItemValue*)NULL,
	"The name of the xApp.");
    config[XAPP_ID] = new Item(
	STRING,'x',"xapp-id","XAPP_ID",true,(ItemValue*)NULL,
	"The ID of the xApp.");
    config[HOST] = new Item(
	STRING,'H',"host","HOST",false,new ItemValue("0.0.0.0"),
	"The local IP on which the RMR service listens.");
    config[PORT] = new Item(
	INTEGER,'p',"port","PORT",false,new ItemValue(4560),
	"The RMR listen port (default 4560)");
    config[LOG_LEVEL] = new Item(
	STRING,'l',"log-level","LOG_LEVEL",false,new ItemValue("warn"),
	"The name of the xApp.");
    config[ADMIN_HOST] = new Item(
	STRING,'A',"admin-host","ADMIN_HOST",false,new ItemValue("0.0.0.0"),
	"The local IP on which the northbound interface listens.");
    config[ADMIN_PORT] = new Item(
	INTEGER,'a',"admin-port","ADMIN_PORT",false,new ItemValue(8000),
	"The northbound interface listen port.");
    config[RMR_NOWAIT] = new Item(
	BOOL,'R',"rmr-nowait","RMR_NOWAIT",false,new ItemValue(false),
	"Do not wait for RMR route established (waits by default).");

    optstr = (char *)calloc(config.size() + 2 + 1,2);
    long_options = (struct option *)calloc(config.size() + 2,
					   sizeof(struct option));
    int i = 0;
    char *optstr_ptr = optstr;
    for (auto it = config.begin(); it != config.end(); ++it) {
	auto name = it->first;
	auto item = it->second;

	sprintf(optstr_ptr,"%c",item->short_option_name);
	++optstr_ptr;
	if (item->type != BOOL) {
	    sprintf(optstr_ptr,":");
	    ++optstr_ptr;
	}

	if (item->long_option_name == NULL)
	    continue;
	long_options[i].name = item->long_option_name;
	if (item->type == BOOL)
	    long_options[i].has_arg = 0;
	else
	    long_options[i].has_arg = 2;
	long_options[i].val = item->short_option_name;
	++i;
    }
    long_options[i].name = "help";
    long_options[i].has_arg = 0;
    long_options[i].val = 'h';
}

nexran::Config::~Config()
{
    free(optstr);
    free(long_options);
}

bool nexran::Config::parseEnv()
{
    for (auto it = config.begin(); it != config.end(); ++it) {
	auto item = it->second;

	if (!item->env_name)
	    continue;

	char *val = getenv(item->env_name);
	if (!val)
	    continue;

	if (item->type == BOOL)
	    item->value.b = not item->value.b;
	else if (item->type == STRING)
	    item->value.s = val;
	else if (item->type == INTEGER)
	    item->value.i = std::atoi(val);
	else
	    return false;

	item->value_set = true;
    }

    return true;
}

bool nexran::Config::parseArgv(int argc,char **argv)
{
    while (true) {
	int optindex = 0;
	char c = getopt_long(argc,argv,optstr,long_options,&optindex);
	if (c == -1)
	    return true;
	if (c == 'h')
	    return false;
	bool found = false;
	for (auto it = config.begin(); it != config.end(); ++it) {
	    auto name = it->first;
	    auto item = it->second;

	    if (c != item->short_option_name)
		continue;

	    if (item->type == BOOL)
		item->value.b = true;
	    else if (item->type == STRING)
		item->value.s = optarg;
	    else if (item->type == INTEGER)
		item->value.i = std::atoi(optarg);
	    else
		return false;

	    item->value_set = true;
	    found = true;
	    break;
	}
	if (!found)
	    return false;
    }

    return true;
}

bool nexran::Config::validate()
{
    for (auto it = config.begin(); it != config.end(); ++it) {
	auto item = it->second;

	if (item->required && !(item->value_set || item->default_value))
	    return false;
    }

    return true;
}

void nexran::Config::usage(const char *progname)
{
    std::printf("Usage: %s [OPTION]\n",progname);
    std::printf("\n");
    std::printf("Mandatory arguments:\n");
    for (auto it = config.begin(); it != config.end(); ++it) {
	auto item = it->second;

	if (!item->required)
	    continue;

	std::printf("  -%c",item->short_option_name);
	if (item->long_option_name)
	    std::printf(", --%s",item->long_option_name);
	std::printf("\t\t%s\n",item->help);
    }
    std::printf("\n");
    std::printf("Optional arguments:\n");
    for (auto it = config.begin(); it != config.end(); ++it) {
	auto item = it->second;

	if (item->required)
	    continue;

	std::printf("  -%c",item->short_option_name);
	if (item->long_option_name)
	    std::printf(", --%s",item->long_option_name);
	std::printf("\t\t%s\n",item->help);
    }
}

nexran::xAppSettings::xAppSettings()
{
	if(config_names_map[XAPP_ID].empty())
		config_names_map[XAPP_ID] = "nexran";
	if(config_names_map[XAPP_NAME].empty())
		config_names_map[XAPP_NAME] = "nexran";
	if(config_names_map[VERSION].empty())
		config_names_map[VERSION] = "0.2.1";
	if(config_names_map[RMR_PORT].empty())
		config_names_map[RMR_PORT] = "4560";
	if(config_names_map[HTTP_PORT].empty())
		config_names_map[HTTP_PORT] = "8000";
	if(config_names_map[CONFIG_FILE].empty())
		config_names_map[CONFIG_FILE] = "/nexran/etc/nexran-config-file.json";	
}

void nexran::xAppSettings::loadSettingsFromEnv()
{
	if (const char *env_xname = std::getenv("XAPP_NAME")){
		config_names_map[XAPP_NAME].assign(env_xname);
		mdclog_write(MDCLOG_INFO,"Xapp Name set to %s from environment variable", config_names_map[XAPP_NAME].c_str());
	}
	if (const char *env_xid = std::getenv("XAPP_ID")){
		config_names_map[XAPP_ID].assign(env_xid);
		mdclog_write(MDCLOG_INFO,"Xapp ID set to %s from environment variable", config_names_map[XAPP_ID].c_str());
	}
	if (const char *env_ports = std::getenv("RMR_PORT")){
		config_names_map[RMR_PORT].assign(env_ports);
		mdclog_write(MDCLOG_INFO,"Ports set to %s from environment variable", config_names_map[RMR_PORT].c_str());
	}
	if (const char *env_config_file = std::getenv("CONFIG_FILE")){
		config_names_map[CONFIG_FILE].assign(env_config_file);
		mdclog_write(MDCLOG_INFO,"Config file set to %s from environment variable", config_names_map[CONFIG_FILE].c_str());
	}
	if (char *env = getenv("RMR_SRC_ID")) {
		config_names_map[RMR_SRC_ID].assign(env);
		mdclog_write(MDCLOG_INFO,"RMR_SRC_ID set to %s from environment variable", config_names_map[RMR_SRC_ID].c_str());
	} else {
		mdclog_write(MDCLOG_ERR, "RMR_SRC_ID env var is not defined");
	}
}

void nexran::xAppSettings::loadxAppDescriptorSettings()
{
	mdclog_write(MDCLOG_INFO, "Loading xApp descriptor file");

	FILE *fp = fopen(config_names_map[CONFIG_FILE].c_str(), "r");
	if (fp == NULL) {
		mdclog_write(MDCLOG_ERR, "unable to open config file %s",
					config_names_map[CONFIG_FILE].c_str());
		return;
	}
	char buffer[4096];
	FileReadStream is(fp, buffer, sizeof(buffer));
	Document doc;
	doc.ParseStream(is);

	if (Value *value = Pointer("/version").Get(doc)) {
		config_names_map[VERSION].assign(value->GetString());
	} else {
		mdclog_write(MDCLOG_WARN, "unable to get version from config file");
	}
	if (Value *value = Pointer("/messaging/ports").Get(doc)) {
		auto array = value->GetArray();
		for (auto &el : array) {
			if (el.HasMember("name") && el.HasMember("port")) {
				std::string name = el["name"].GetString();

				if (name.compare("rmr-data") == 0) 
					config_names_map[RMR_PORT].assign(std::to_string(el["port"].GetInt()));
				else if (name.compare("nbi") == 0) {
					config_names_map[HTTP_PORT].assign(std::to_string(el["port"].GetInt()));
				}
			}
		}
	} else {
		mdclog_write(MDCLOG_WARN, "unable to get ports from config file");
	}

	StringBuffer outbuf;
	outbuf.Clear();
	Writer<StringBuffer> writer(outbuf);
	doc.Accept(writer);
	config_names_map[CONFIG_STR].assign(outbuf.GetString());

	fclose(fp);
}
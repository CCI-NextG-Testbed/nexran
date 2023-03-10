#ifndef _NEXRAN_CONFIG_H_
#define _NEXRAN_CONFIG_H_

#include "getopt.h"
#include <map>
#include <string>
#include <list>
#include <memory>
#include "mdclog/mdclog.h"

namespace nexran {

/**
 * We don't want to drag in boost just for parameters.  What more is
 * there to say?
 */
class Config {
 public:
    enum ItemName {
	XAPP_NAME = 1,
	XAPP_ID,
	HOST,
	PORT,
	LOG_LEVEL,
	ADMIN_HOST,
	ADMIN_PORT,
	RMR_NOWAIT,
	__MAX__
    };
    enum ItemType {
	BOOL = 1,
	STRING,
	INTEGER,
    };
    union ItemValue {
	ItemValue(): s(NULL) {};
	ItemValue(bool b_): b(b_) {};
	ItemValue(const char *s_): s(s_) {};
	ItemValue(int i_): i(i_) {};
	bool b;
	const char *s;
	int i;
    };
    class Item {
     public:
	Item(ItemType type_,char short_option_name_,
	     const char *long_option_name_,const char *env_name_,
	     bool required_,ItemValue *default_value_,const char *help_)
	  : type(type_),short_option_name(short_option_name_),
	    long_option_name(long_option_name_),env_name(env_name_),
	    required(required_),help(help_),default_value(default_value_),
	    value_set(false) {};
	virtual ~Item() = default;
	ItemType type;
	char short_option_name;
	const char *long_option_name;
	const char *env_name;
	bool required;
	const char *help;
	ItemValue *default_value;
	bool value_set;
	ItemValue value;
    };
    Config();
    virtual ~Config();
    bool parseArgv(int argc,char **argv);
    bool parseEnv();
    bool validate();
    void usage(const char *progname);
    ItemValue *operator[](const ItemName& name) {
	if (config[name]->value_set)
	    return &config[name]->value;
	else
	    return config[name]->default_value;
    };

 private:
    std::map<ItemName,Item *> config;
    struct option *long_options;
    char *optstr;
    
};

class xAppSettings {
public:
	typedef enum {
		XAPP_ID,
		XAPP_NAME,
		VERSION,
		RMR_SRC_ID,
		RMR_PORT,
		HTTP_PORT,
		CONFIG_FILE,
		CONFIG_STR
	} config_name;

	xAppSettings();
	void loadSettingsFromEnv();
	void loadxAppDescriptorSettings();
	std::string& operator[](const config_name& value){
		return config_names_map[value];
	}
private:
	typedef std::map<config_name, std::string> ConfigMap;
	ConfigMap config_names_map;
};

}

#endif /* _NEXRAN_CONFIG_H_ */

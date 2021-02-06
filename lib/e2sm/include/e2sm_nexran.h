#ifndef _E2SM_NEXRAN_H_
#define _E2SM_NEXRAN_H_

#include <list>
#include <map>
#include <string>
#include <cstdint>

#include "e2ap.h"
#include "e2sm.h"

namespace e2sm
{
namespace nexran
{

class ProportionalAllocationPolicy
{
 public:
    ProportionalAllocationPolicy()
	: share(1024) {};
    ProportionalAllocationPolicy(int share_)
	: share(share_) {};
    virtual ~ProportionalAllocationPolicy() = default;

    int share;
};

class SliceConfig
{
 public:
    SliceConfig(std::string &name_,ProportionalAllocationPolicy *policy_)
	: name(name_),policy(policy_) {};
    virtual ~SliceConfig() = default;

    std::string name;
    ProportionalAllocationPolicy *policy;
};

class SliceStatus
{
 public:
    SliceStatus(std::string &name_,ProportionalAllocationPolicy *policy_,
		std::list<std::string> &imsis_)
	: name(name_),policy(policy_),imsis(imsis_) {};
    virtual ~SliceStatus() = default;

    std::string name;
    ProportionalAllocationPolicy *policy;
    std::list<std::string> imsis;
};

class SliceConfigRequest : public e2sm::Control
{
 public:
    SliceConfigRequest(SliceConfig *slice_config) { configs.push_back(slice_config); };
    SliceConfigRequest(std::list<SliceConfig *> &configs_)
	: configs(configs_) {};
    virtual ~SliceConfigRequest() = default;

    virtual bool encode();

 private:
    std::list<SliceConfig *> configs;
};

class SliceDeleteRequest : public e2sm::Control
{
 public:
    SliceDeleteRequest(std::string &name) { names.push_back(name); };
    SliceDeleteRequest(std::list<std::string> &names_)
	: names(names_) {};
    virtual ~SliceDeleteRequest() = default;

    virtual bool encode();

 private:
    std::list<std::string> names;
};

class SliceStatusRequest : public e2sm::Control
{
 public:
    SliceStatusRequest(std::string &name) { names.push_back(name); };
    SliceStatusRequest(std::list<std::string> &names_)
	: names(names_) {};
    virtual ~SliceStatusRequest() = default;

    virtual bool encode();

 private:
    std::list<std::string> names;
};

class SliceStatusReport : public e2sm::Indication
{
 public:
    SliceStatusReport(SliceStatus *status) { statuses.push_back(status); };
    SliceStatusReport(std::list<SliceStatus *> &statuses_)
	: statuses(statuses_) {};
    virtual ~SliceStatusReport() = default;

    virtual bool encode();

 private:
    std::list<SliceStatus *> statuses;
};

class SliceUeBindRequest : public e2sm::Control
{
 public:
    SliceUeBindRequest(std::string &slice_,std::string imsi_)
        : slice(slice_) { imsis.push_back(imsi_); };
    SliceUeBindRequest(std::string &slice_,std::list<std::string> imsis_)
        : slice(slice_),imsis(imsis_) {};
    virtual ~SliceUeBindRequest() = default;

    virtual bool encode();

 private:
    std::string slice;
    std::list<std::string> imsis;
};

class SliceUeUnbindRequest : public e2sm::Control
{
 public:
    SliceUeUnbindRequest(std::string &slice_,std::string imsi_)
        : slice(slice_) { imsis.push_back(imsi_); };
    SliceUeUnbindRequest(std::string &slice_,std::list<std::string> imsis_)
        : slice(slice_),imsis(imsis_) {};
    virtual ~SliceUeUnbindRequest() = default;

    virtual bool encode();

 private:
    std::string slice;
    std::list<std::string> imsis;
};

class NexRANModel : public e2sm::Model
{
 public:
    NexRANModel()
	: e2sm::Model("ORAN-E2SM-NEXRAN","1.3.6.1.4.1.1.1.2.100") {};
    virtual ~NexRANModel() = default;
    virtual int init() { return 0; };
    virtual void stop() {};
};

}
}

#endif /* _E2SM_NEXRAN_H_ */

#ifndef _E2SM_ZYLINIUM_H_
#define _E2SM_ZYLINIUM_H_

#include <list>
#include <map>
#include <string>
#include <cstdint>

#include "e2ap.h"
#include "e2sm.h"

namespace e2sm
{
namespace zylinium
{

class BlockedMask
{
public:
    BlockedMask() {};
    BlockedMask(const std::string& dl_rbg_mask_,const std::string& ul_prb_mask_)
	: dl_rbg_mask(dl_rbg_mask_),ul_prb_mask(ul_prb_mask_) {};

    std::string dl_rbg_mask;
    std::string ul_prb_mask;
};

class MaskConfigRequest : public e2sm::Control
{
 public:
    MaskConfigRequest(e2sm::Model *model_,BlockedMask *mask_)
	: e2sm::Control(model_),mask(mask_) {};
    virtual ~MaskConfigRequest() = default;

    virtual bool encode();

 private:
    BlockedMask *mask;
};

class MaskStatusRequest : public e2sm::Control
{
 public:
    MaskStatusRequest(e2sm::Model *model_)
	: e2sm::Control(model_) {};
    virtual ~MaskStatusRequest() = default;

    virtual bool encode();
};

class MaskStatusReport : public e2sm::Indication
{
 public:
    MaskStatusReport(e2sm::Model *model_,BlockedMask *mask_)
	: e2sm::Indication(model_),mask(mask_) {};
    virtual ~MaskStatusReport() = default;

    bool encode() { return false; }

 private:
    BlockedMask *mask;
};

class MaskStatusIndication : public e2sm::Indication
{
 public:
    MaskStatusIndication(e2sm::Model *model_)
	: e2sm::Indication(model_) {};
    MaskStatusIndication(e2sm::Model *model_,MaskStatusReport *report_)
	: report(report_),e2sm::Indication(model_) {};
    virtual ~MaskStatusIndication() { delete report; };
    virtual bool encode() { return false; };

    MaskStatusReport *report;
};

class MaskStatusControlOutcome : public e2sm::ControlOutcome
{
 public:
    MaskStatusControlOutcome(e2sm::Model *model_,BlockedMask *mask_)
	: e2sm::ControlOutcome(model_),mask(mask_) {};
    virtual ~MaskStatusControlOutcome() = default;

    bool encode() { return false; };

 private:
    BlockedMask *mask;
};

class EventTrigger : public e2sm::EventTrigger
{
 public:
    EventTrigger(e2sm::Model *model_)
	: e2sm::EventTrigger(model_) {};
    virtual ~EventTrigger() = default;

    virtual bool encode();
};

class AgentInterface
{
 public:
    virtual bool handle(e2sm::zylinium::MaskStatusIndication *ind) = 0;
};

class ZyliniumModel : public e2sm::Model
{
 public:
    ZyliniumModel(AgentInterface *agent_if_)
	: agent_if(agent_if_),e2sm::Model("ORAN-E2SM-ZYLINIUM","1.3.6.1.4.1.1.1.2.999") {};
    virtual ~ZyliniumModel() = default;
    virtual int init() { return 0; };
    virtual void stop() {};

    Indication *decode(e2ap::Indication *ind,
		       unsigned char *header,ssize_t header_len,
		       unsigned char *message,ssize_t message_len);
    ControlOutcome *decode(e2ap::ControlAck *ack,
			   unsigned char *outcome,ssize_t outcome_len);
    ControlOutcome *decode(e2ap::ControlFailure *failure,
			   unsigned char *outcome,ssize_t outcome_len);

 protected:
    AgentInterface *agent_if;
};

}
}

#endif /* _E2SM_KPM_H_ */

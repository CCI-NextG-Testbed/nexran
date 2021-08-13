
#include <cstring>

#include "mdclog/mdclog.h"

#include "e2ap.h"
#include "e2sm.h"
#include "e2sm_internal.h"
#include "e2sm_zylinium.h"

#include "E2SM_ZYLINIUM_RANfunction-Description.h"
#include "E2SM_ZYLINIUM_E2SM-Zylinium-ControlHeader.h"
#include "E2SM_ZYLINIUM_E2SM-Zylinium-ControlMessage.h"
#include "E2SM_ZYLINIUM_MaskConfigRequest.h"
#include "E2SM_ZYLINIUM_E2SM-Zylinium-EventTriggerDefinition.h"
#include "E2SM_ZYLINIUM_E2SM-Zylinium-ControlOutcome.h"
#include "E2SM_ZYLINIUM_E2SM-Zylinium-IndicationHeader.h"
#include "E2SM_ZYLINIUM_E2SM-Zylinium-IndicationMessage.h"
#include "E2SM_ZYLINIUM_BlockedMask.h"

namespace e2sm
{
namespace zylinium
{

static BlockedMask *decode_mask_status_report(E2SM_ZYLINIUM_MaskStatusReport *report)
{
    std::string dl_rbg_mask;
    std::string ul_prb_mask;

    if (report->blockedMask.blockedDLRBGMask.buf
	&& report->blockedMask.blockedDLRBGMask.size > 0)
	dl_rbg_mask = std::string(
	    (char *)report->blockedMask.blockedDLRBGMask.buf,0,
	    report->blockedMask.blockedDLRBGMask.size);
    if (report->blockedMask.blockedULPRBMask.buf
	&& report->blockedMask.blockedULPRBMask.size > 0)
	ul_prb_mask = std::string(
	    (char *)report->blockedMask.blockedULPRBMask.buf,0,
	    report->blockedMask.blockedULPRBMask.size);

    return new BlockedMask(dl_rbg_mask,ul_prb_mask);
}

Indication *ZyliniumModel::decode(
    e2ap::Indication *ind,
    unsigned char *header,ssize_t header_len,
    unsigned char *message,ssize_t message_len)
{
    E2SM_ZYLINIUM_E2SM_Zylinium_IndicationHeader_t h;
    E2SM_ZYLINIUM_E2SM_Zylinium_IndicationMessage_t m;
    void *ptr;

    memset(&h,0,sizeof(h));
    memset(&m,0,sizeof(m));
    
    asn_dec_rval_t dres;
    bool have_header = false,have_message = false;

    if (header && header_len > 0) {
	ptr = &h;
	dres = aper_decode(
	    NULL,&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_IndicationHeader,
	    &ptr,header,header_len,0,0);
	if (dres.code != RC_OK) {
	    mdclog_write(MDCLOG_ERR,"failed to decode zylinium indication header (len %lu, code %d)\n",
			 header_len,dres.code);
	    return NULL;
	}
	have_header = true;
	E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_IndicationHeader,&h);
    }
    if (message && message_len > 0) {
	ptr = &m;
	dres = aper_decode(
	    NULL,&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_IndicationMessage,
	    &ptr,message,message_len,0,0);
	if (dres.code != RC_OK) {
	    mdclog_write(MDCLOG_ERR,"failed to decode zylinium indication message (len %lu, code %d)\n",
			 message_len,dres.code);
	    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_IndicationHeader,&h);
	    return NULL;
	}
	have_message = true;
	E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_IndicationMessage,&m);
    }
    if (!have_header) {
	mdclog_write(MDCLOG_ERR,"missing zylinium indication header; aborting\n");
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_IndicationHeader,&h);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_IndicationMessage,&m);
	return NULL;
    }
    if (!have_message) {
	mdclog_write(MDCLOG_ERR,"missing zylinium indication message; aborting\n");
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_IndicationHeader,&h);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_IndicationMessage,&m);
	return NULL;
    }

    if (m.present != E2SM_ZYLINIUM_E2SM_Zylinium_IndicationMessage_PR_maskStatusReport) {
	mdclog_write(MDCLOG_ERR,"unsupported zylinium indication message (%d); aborting\n",
		     m.present);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_IndicationHeader,&h);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_IndicationMessage,&m);
	return NULL;
    }

    // XXX: meid

    BlockedMask *mask = decode_mask_status_report(&m.choice.maskStatusReport);

    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_IndicationHeader,&h);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_IndicationMessage,&m);

    return new MaskStatusReport(this,mask);
}

ControlOutcome *ZyliniumModel::decode(
    e2ap::ControlAck *ack,
    unsigned char *outcome,ssize_t outcome_len)
{
    if (!outcome || outcome_len <= 0)
	return NULL;

    E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome_t o;
    memset(&o,0,sizeof(o));

    void *ptr = &o;
    asn_dec_rval_t dres = aper_decode(
        NULL,&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome,
	&ptr,outcome,outcome_len,0,0);
    if (dres.code != RC_OK) {
	mdclog_write(MDCLOG_ERR,"failed to decode zylinium control outcome (len %lu, code %d)\n",
		     outcome_len,dres.code);
	return NULL;
    }
    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome,&o);
    if (o.present != E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome_PR_controlOutcomeFormat1) {
	mdclog_write(MDCLOG_ERR,"unsupported zylinium control outcome (%d); aborting\n",
		     o.present);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome,&o);
	return NULL;
    }
    if (o.choice.controlOutcomeFormat1.present != E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome_Format1_PR_maskStatusReport) {
	mdclog_write(MDCLOG_ERR,"unsupported zylinium control outcome format1 (%d); aborting\n",
		     o.choice.controlOutcomeFormat1.present);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome,&o);
	return NULL;
    }

    BlockedMask *mask = decode_mask_status_report(&o.choice.controlOutcomeFormat1.choice.maskStatusReport);

    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome,&o);

    return new MaskStatusControlOutcome(this,mask);
}

ControlOutcome *ZyliniumModel::decode(
    e2ap::ControlFailure *failure,
    unsigned char *outcome,ssize_t outcome_len)
{
    if (!outcome || outcome_len <= 0)
	return NULL;

    E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome_t o;
    memset(&o,0,sizeof(o));

    void *ptr = &o;
    asn_dec_rval_t dres = aper_decode(
        NULL,&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome,
	&ptr,outcome,outcome_len,0,0);
    if (dres.code != RC_OK) {
	mdclog_write(MDCLOG_ERR,"failed to decode zylinium control outcome (len %lu, code %d)\n",
		     outcome_len,dres.code);
	return NULL;
    }
    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome,&o);
    if (o.present != E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome_PR_controlOutcomeFormat1) {
	mdclog_write(MDCLOG_ERR,"unsupported zylinium control outcome (%d); aborting\n",
		     o.present);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome,&o);
	return NULL;
    }
    if (o.choice.controlOutcomeFormat1.present != E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome_Format1_PR_maskStatusReport) {
	mdclog_write(MDCLOG_ERR,"unsupported zylinium control outcome format1 (%d); aborting\n",
		     o.choice.controlOutcomeFormat1.present);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome,&o);
	return NULL;
    }

    BlockedMask *mask = decode_mask_status_report(
        &o.choice.controlOutcomeFormat1.choice.maskStatusReport);

    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlOutcome,&o);

    return new MaskStatusControlOutcome(this,mask);
}

bool MaskConfigRequest::encode()
{
    if (encoded)
	return true;

    E2SM_ZYLINIUM_E2SM_Zylinium_ControlHeader_t h;
    E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage_t m;

    memset(&h,0,sizeof(h));
    memset(&m,0,sizeof(m));

    h.present = E2SM_ZYLINIUM_E2SM_Zylinium_ControlHeader_PR_controlHeaderFormat1;
    h.choice.controlHeaderFormat1.controlMessageId = \
	E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage_Id_maskConfigRequest;

    m.present = E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage_PR_controlMessageFormat1;
    m.choice.controlMessageFormat1.present = \
	E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage_Format1_PR_maskConfigRequest;

    E2SM_ZYLINIUM_BlockedMask_t *bm = \
	&m.choice.controlMessageFormat1.choice.maskConfigRequest.blockedMask;
    bm->blockedDLRBGMask.size = strlen(mask->dl_rbg_mask.c_str());
    bm->blockedDLRBGMask.buf = (uint8_t *)malloc(bm->blockedDLRBGMask.size);
    memcpy(bm->blockedDLRBGMask.buf,mask->dl_rbg_mask.c_str(),
	   bm->blockedDLRBGMask.size);
    bm->blockedULPRBMask.size = strlen(mask->ul_prb_mask.c_str());
    bm->blockedULPRBMask.buf = (uint8_t *)malloc(bm->blockedULPRBMask.size);
    memcpy(bm->blockedULPRBMask.buf,mask->ul_prb_mask.c_str(),
	   bm->blockedULPRBMask.size);

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlHeader,&h);

    ssize_t len = e2sm::encode(&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlHeader,NULL,&h,&header);
    if (len < 0) {
	header = NULL;
	header_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlHeader,&h);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage,&m);
	return false;
    }
    header_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlHeader,&h);

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage,&m);

    len = e2sm::encode(&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage,NULL,&m,&message);
    if (len < 0) {
	free(header);
	header = NULL;
	header_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage,&m);
	return false;
    }
    message_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage,&m);

    encoded = true;

    return true;
}

bool MaskStatusRequest::encode()
{
    if (encoded)
	return true;

    E2SM_ZYLINIUM_E2SM_Zylinium_ControlHeader_t h;
    E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage_t m;

    memset(&h,0,sizeof(h));
    memset(&m,0,sizeof(m));

    h.present = E2SM_ZYLINIUM_E2SM_Zylinium_ControlHeader_PR_controlHeaderFormat1;
    h.choice.controlHeaderFormat1.controlMessageId = \
	E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage_Id_maskStatusRequest;

    m.present = E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage_PR_controlMessageFormat1;
    m.choice.controlMessageFormat1.present = \
	E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage_Format1_PR_maskStatusRequest;

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlHeader,&h);

    ssize_t len = e2sm::encode(&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlHeader,NULL,&h,&header);
    if (len < 0) {
	header = NULL;
	header_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlHeader,&h);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage,&m);
	return 1;
    }
    header_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlHeader,&h);

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage,&m);

    len = e2sm::encode(&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage,NULL,&m,&message);
    if (len < 0) {
	free(header);
	header = NULL;
	header_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage,&m);
	return 1;
    }
    message_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_ControlMessage,&m);

    encoded = true;
    return true;
}

bool EventTrigger::encode()
{
    if (encoded)
	return true;

    E2SM_ZYLINIUM_E2SM_Zylinium_EventTriggerDefinition_t td;

    memset(&td,0,sizeof(td));

    td.present = E2SM_ZYLINIUM_E2SM_Zylinium_EventTriggerDefinition_PR_ranEventDefinition;

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_EventTriggerDefinition,&td);

    ssize_t len = e2sm::encode(&asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_EventTriggerDefinition,NULL,&td,&buf);
    if (len < 0) {
	buf = NULL;
	buf_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_EventTriggerDefinition,&td);
	return false;
    }
    buf_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_ZYLINIUM_E2SM_Zylinium_EventTriggerDefinition,&td);

    encoded = true;
    return true;
}

}
}

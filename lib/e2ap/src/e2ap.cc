
#include "e2ap.h"
#include "e2sm.h"

#include "E2AP_E2AP-PDU.h"
#include "E2AP_InitiatingMessage.h"
#include "E2AP_SuccessfulOutcome.h"
#include "E2AP_UnsuccessfulOutcome.h"
#include "E2AP_ProtocolIE-ID.h"
#include "E2AP_ProtocolIE-Field.h"
#include "E2AP_Cause.h"

namespace e2ap
{

bool xer_print = true;

static RanFunctionId next_ran_function_id = 0;

RanFunctionId get_next_ran_function_id()
{
    return next_ran_function_id++;
}

ssize_t encode(
    const struct asn_TYPE_descriptor_s *td,
    const asn_per_constraints_t *constraints,void *sptr,
    unsigned char **buf)
{
    ssize_t encoded;

    encoded = aper_encode_to_new_buffer(td,constraints,sptr,(void **)buf);
    if (encoded < 0)
	return -1;

    ASN_STRUCT_FREE_CONTENTS_ONLY((*td),sptr);

    return encoded;
}

ssize_t encode_pdu(E2AP_E2AP_PDU_t *pdu,unsigned char **buf,ssize_t *len)
{
    ssize_t encoded;

    encoded = encode(&asn_DEF_E2AP_E2AP_PDU,0,pdu,buf);
    if (encoded < 0)
	return -1;

    *len = encoded;

    return encoded;
}


bool E2AP::init()
{
    return true;
}

bool ControlRequest::encode()
{
    E2AP_E2AP_PDU_t pdu;
    E2AP_RICcontrolRequest_t *req;
    E2AP_RICcontrolRequest_IEs_t *ie;
    unsigned char *iebuf;
    size_t iebuflen;

    if (encoded)
	return true;

    memset(&pdu,0,sizeof(pdu));
    pdu.present = E2AP_E2AP_PDU_PR_initiatingMessage;
    pdu.choice.initiatingMessage.procedureCode = E2AP_ProcedureCode_id_RICcontrol;
    pdu.choice.initiatingMessage.criticality = E2AP_Criticality_reject;
    pdu.choice.initiatingMessage.value.present = E2AP_InitiatingMessage__value_PR_RICcontrolRequest;
    req = &pdu.choice.initiatingMessage.value.choice.RICcontrolRequest;

    ie = (E2AP_RICcontrolRequest_IEs_t *)calloc(1,sizeof(*ie));
    ie->id = E2AP_ProtocolIE_ID_id_RICrequestID;
    ie->criticality = E2AP_Criticality_reject;
    ie->value.present = E2AP_RICcontrolRequest_IEs__value_PR_RICrequestID;
    ie->value.choice.RICrequestID.ricRequestorID = requestor_id;
    ie->value.choice.RICrequestID.ricInstanceID = instance_id;
    ASN_SEQUENCE_ADD(&req->protocolIEs.list,ie);

    ie = (E2AP_RICcontrolRequest_IEs_t *)calloc(1,sizeof(*ie));
    ie->id = E2AP_ProtocolIE_ID_id_RANfunctionID;
    ie->criticality = E2AP_Criticality_reject;
    ie->value.present = E2AP_RICcontrolRequest_IEs__value_PR_RANfunctionID;
    ie->value.choice.RANfunctionID = function_id;
    ASN_SEQUENCE_ADD(&req->protocolIEs.list,ie);

    iebuf = model->get_call_process_id();
    iebuflen = model->get_call_process_id_len();
    if (iebuf && iebuflen > 0) {
	ie = (E2AP_RICcontrolRequest_IEs_t *)calloc(1,sizeof(*ie));
	ie->id = E2AP_ProtocolIE_ID_id_RICcallProcessID;
	ie->criticality = E2AP_Criticality_reject;
	ie->value.present = E2AP_RICcontrolRequest_IEs__value_PR_RICcallProcessID;
	ie->value.choice.RICcallProcessID.buf = (uint8_t *)malloc(iebuflen);
	memcpy(ie->value.choice.RICcallProcessID.buf,iebuf,iebuflen);
	ie->value.choice.RICcallProcessID.size = iebuflen;
	ASN_SEQUENCE_ADD(&req->protocolIEs.list,ie);
    }

    iebuf = model->get_header();
    iebuflen = model->get_header_len();
    ie = (E2AP_RICcontrolRequest_IEs_t *)calloc(1,sizeof(*ie));
    ie->id = E2AP_ProtocolIE_ID_id_RICcontrolHeader;
    ie->criticality = E2AP_Criticality_reject;
    ie->value.present = E2AP_RICcontrolRequest_IEs__value_PR_RICcontrolHeader;
    ie->value.choice.RICcontrolHeader.buf = (uint8_t *)malloc(iebuflen);
    memcpy(ie->value.choice.RICcontrolHeader.buf,iebuf,iebuflen);
    ie->value.choice.RICcontrolHeader.size = iebuflen;
    ASN_SEQUENCE_ADD(&req->protocolIEs.list,ie);

    iebuf = model->get_message();
    iebuflen = model->get_message_len();
    ie = (E2AP_RICcontrolRequest_IEs_t *)calloc(1,sizeof(*ie));
    ie->id = E2AP_ProtocolIE_ID_id_RICcontrolMessage;
    ie->criticality = E2AP_Criticality_reject;
    ie->value.present = E2AP_RICcontrolRequest_IEs__value_PR_RICcontrolMessage;
    ie->value.choice.RICcontrolMessage.buf = (uint8_t *)malloc(iebuflen);
    memcpy(ie->value.choice.RICcontrolMessage.buf,iebuf,iebuflen);
    ie->value.choice.RICcontrolMessage.size = iebuflen;
    ASN_SEQUENCE_ADD(&req->protocolIEs.list,ie);

    ie = (E2AP_RICcontrolRequest_IEs_t *)calloc(1,sizeof(*ie));
    ie->id = E2AP_ProtocolIE_ID_id_RICcontrolAckRequest;
    ie->criticality = E2AP_Criticality_reject;
    ie->value.present = E2AP_RICcontrolRequest_IEs__value_PR_RICcontrolAckRequest;
    ie->value.choice.RICcontrolAckRequest = (E2AP_RICcontrolAckRequest_t)ack_request;
    ASN_SEQUENCE_ADD(&req->protocolIEs.list,ie);

    E2AP_XER_PRINT(NULL,&asn_DEF_E2AP_E2AP_PDU,&pdu);

    if (encode_pdu(&pdu,&buf,&len) < 0) {
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2AP_E2AP_PDU,&pdu);
	return false;
    }

    /*
    E2AP_E2AP_PDU_t pdud;
    memset(&pdud,0,sizeof(pdud));
    if (e2ap_decode_pdu(&pdud,*buffer,*len) < 0) {
        E2AP_WARN("Failed to encode E2setupRequest\n");
    }
    */

    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2AP_E2AP_PDU,&pdu);

    encoded = true;
    return true;
}

}

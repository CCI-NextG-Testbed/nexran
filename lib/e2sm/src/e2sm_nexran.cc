
#include <cstring>

#include "e2ap.h"
#include "e2sm.h"
#include "e2sm_internal.h"
#include "e2sm_nexran.h"

#include "E2SM_NEXRAN_RANfunction-Description.h"
#include "E2SM_NEXRAN_E2SM-NexRAN-ControlHeader.h"
#include "E2SM_NEXRAN_E2SM-NexRAN-ControlMessage.h"
#include "E2SM_NEXRAN_SliceConfigRequest.h"
#include "E2SM_NEXRAN_SliceConfig.h"
#include "E2SM_NEXRAN_SchedPolicy.h"
#include "E2SM_NEXRAN_SliceDeleteRequest.h"

namespace e2sm
{
namespace nexran
{

bool SliceConfigRequest::encode()
{
    if (encoded)
	return true;

    E2SM_NEXRAN_E2SM_NexRAN_ControlHeader_t h;
    E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_t m;

    memset(&h,0,sizeof(h));
    memset(&m,0,sizeof(m));

    h.present = E2SM_NEXRAN_E2SM_NexRAN_ControlHeader_PR_controlHeaderFormat1;
    h.choice.controlHeaderFormat1.controlMessageId = \
	E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_Id_sliceConfigRequest;

    m.present = E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_PR_controlMessageFormat1;
    m.choice.controlMessageFormat1.present = \
	E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_Format1_PR_sliceConfigRequest;
    for (auto it = configs.begin(); it != configs.end(); ++it) {
	E2SM_NEXRAN_SliceConfig_t *ie = (E2SM_NEXRAN_SliceConfig_t *)calloc(1,sizeof(*ie));
	ie->sliceName.size = strlen((*it)->name.c_str());
	ie->sliceName.buf = (uint8_t *)malloc(ie->sliceName.size + 1);
	strcpy((char *)ie->sliceName.buf,(*it)->name.c_str());

	ie->schedPolicy.present = E2SM_NEXRAN_SchedPolicy_PR_proportionalAllocationPolicy;
	ie->schedPolicy.choice.proportionalAllocationPolicy.share = (*it)->policy->share;
	ASN_SEQUENCE_ADD(&m.choice.controlMessageFormat1.choice.sliceConfigRequest.sliceConfigList.list,ie);
    }

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);

    ssize_t len = e2sm::encode(&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,NULL,&h,&header);
    if (len < 0) {
	header = NULL;
	header_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);
	return 1;
    }
    header_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);

    len = e2sm::encode(&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,NULL,&m,&message);
    if (len < 0) {
	free(header);
	header = NULL;
	header_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);
	return 1;
    }
    message_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);

    encoded = true;
    return true;
}

bool SliceDeleteRequest::encode()
{
    if (encoded)
	return true;

    E2SM_NEXRAN_E2SM_NexRAN_ControlHeader_t h;
    E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_t m;

    memset(&h,0,sizeof(h));
    memset(&m,0,sizeof(m));

    h.present = E2SM_NEXRAN_E2SM_NexRAN_ControlHeader_PR_controlHeaderFormat1;
    h.choice.controlHeaderFormat1.controlMessageId = \
	E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_Id_sliceDeleteRequest;

    m.present = E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_PR_controlMessageFormat1;
    m.choice.controlMessageFormat1.present = \
	E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_Format1_PR_sliceDeleteRequest;
    for (auto it = names.begin(); it != names.end(); ++it) {
	E2SM_NEXRAN_SliceName_t *ie = (E2SM_NEXRAN_SliceName_t *)calloc(1,sizeof(*ie));
	ie->size = strlen((*it).c_str());
	ie->buf = (uint8_t *)malloc(ie->size + 1);
	strcpy((char *)ie->buf,(*it).c_str());
	ASN_SEQUENCE_ADD(&m.choice.controlMessageFormat1.choice.sliceDeleteRequest.sliceNameList.list,ie);
    }

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);

    ssize_t len = e2sm::encode(&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,NULL,&h,&header);
    if (len < 0) {
	header = NULL;
	header_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);
	return 1;
    }
    header_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);

    len = e2sm::encode(&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,NULL,&m,&message);
    if (len < 0) {
	free(header);
	header = NULL;
	header_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);
	return 1;
    }
    message_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);

    encoded = true;
    return true;
}

bool SliceStatusRequest::encode()
{
    if (encoded)
	return true;

    E2SM_NEXRAN_E2SM_NexRAN_ControlHeader_t h;
    E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_t m;

    memset(&h,0,sizeof(h));
    memset(&m,0,sizeof(m));

    h.present = E2SM_NEXRAN_E2SM_NexRAN_ControlHeader_PR_controlHeaderFormat1;
    h.choice.controlHeaderFormat1.controlMessageId = \
	E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_Id_sliceStatusRequest;

    m.present = E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_PR_controlMessageFormat1;
    m.choice.controlMessageFormat1.present = \
	E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_Format1_PR_sliceStatusRequest;
    for (auto it = names.begin(); it != names.end(); ++it) {
	E2SM_NEXRAN_SliceName_t *ie = (E2SM_NEXRAN_SliceName_t *)calloc(1,sizeof(*ie));
	ie->size = strlen(it->c_str());
	ie->buf = (uint8_t *)malloc(ie->size + 1);
	strcpy((char *)ie->buf,it->c_str());
	ASN_SEQUENCE_ADD(&m.choice.controlMessageFormat1.choice.sliceStatusRequest.sliceNameList.list,ie);
    }

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);

    ssize_t len = e2sm::encode(&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,NULL,&h,&header);
    if (len < 0) {
	header = NULL;
	header_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);
	return 1;
    }
    header_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);

    len = e2sm::encode(&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,NULL,&m,&message);
    if (len < 0) {
	free(header);
	header = NULL;
	header_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);
	return 1;
    }
    message_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);

    encoded = true;
    return true;
}

bool SliceUeBindRequest::encode()
{
    if (encoded)
	return true;

    E2SM_NEXRAN_E2SM_NexRAN_ControlHeader_t h;
    E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_t m;

    memset(&h,0,sizeof(h));
    memset(&m,0,sizeof(m));

    h.present = E2SM_NEXRAN_E2SM_NexRAN_ControlHeader_PR_controlHeaderFormat1;
    h.choice.controlHeaderFormat1.controlMessageId = \
	E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_Id_sliceUeBindRequest;

    m.present = E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_PR_controlMessageFormat1;
    m.choice.controlMessageFormat1.present = \
	E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_Format1_PR_sliceUeBindRequest;
    m.choice.controlMessageFormat1.choice.sliceUeBindRequest.sliceName.size = \
	strlen(slice.c_str());
    m.choice.controlMessageFormat1.choice.sliceUeBindRequest.sliceName.buf = \
	(uint8_t *)malloc(strlen(slice.c_str()));
    memcpy(m.choice.controlMessageFormat1.choice.sliceUeBindRequest.sliceName.buf,
	   slice.c_str(),strlen(slice.c_str()));
    for (auto it = imsis.begin(); it != imsis.end(); ++it) {
	E2SM_NEXRAN_IMSI_t *ie = (E2SM_NEXRAN_IMSI_t *)calloc(1,sizeof(*ie));
	ie->size = strlen(it->c_str());
	ie->buf = (uint8_t *)malloc(ie->size + 1);
	strcpy((char *)ie->buf,it->c_str());
	ASN_SEQUENCE_ADD(&m.choice.controlMessageFormat1.choice.sliceUeBindRequest.imsiList.list,ie);
    }

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);

    ssize_t len = e2sm::encode(&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,NULL,&h,&header);
    if (len < 0) {
	header = NULL;
	header_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);
	return 1;
    }
    header_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);

    len = e2sm::encode(&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,NULL,&m,&message);
    if (len < 0) {
	free(header);
	header = NULL;
	header_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);
	return 1;
    }
    message_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);

    encoded = true;
    return true;
}

bool SliceUeUnbindRequest::encode()
{
    if (encoded)
	return true;

    E2SM_NEXRAN_E2SM_NexRAN_ControlHeader_t h;
    E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_t m;

    memset(&h,0,sizeof(h));
    memset(&m,0,sizeof(m));

    h.present = E2SM_NEXRAN_E2SM_NexRAN_ControlHeader_PR_controlHeaderFormat1;
    h.choice.controlHeaderFormat1.controlMessageId = \
	E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_Id_sliceUeUnbindRequest;

    m.present = E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_PR_controlMessageFormat1;
    m.choice.controlMessageFormat1.present = \
	E2SM_NEXRAN_E2SM_NexRAN_ControlMessage_Format1_PR_sliceUeUnbindRequest;
    m.choice.controlMessageFormat1.choice.sliceUeUnbindRequest.sliceName.size = \
	strlen(slice.c_str());
    m.choice.controlMessageFormat1.choice.sliceUeUnbindRequest.sliceName.buf = \
	(uint8_t *)malloc(strlen(slice.c_str()));
    memcpy(m.choice.controlMessageFormat1.choice.sliceUeUnbindRequest.sliceName.buf,
	   slice.c_str(),strlen(slice.c_str()));
    for (auto it = imsis.begin(); it != imsis.end(); ++it) {
	E2SM_NEXRAN_IMSI_t *ie = (E2SM_NEXRAN_IMSI_t *)calloc(1,sizeof(*ie));
	ie->size = strlen(it->c_str());
	ie->buf = (uint8_t *)malloc(ie->size + 1);
	strcpy((char *)ie->buf,it->c_str());
	ASN_SEQUENCE_ADD(&m.choice.controlMessageFormat1.choice.sliceUeUnbindRequest.imsiList.list,ie);
    }

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);

    ssize_t len = e2sm::encode(&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,NULL,&h,&header);
    if (len < 0) {
	header = NULL;
	header_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);
	return 1;
    }
    header_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlHeader,&h);

    E2SM_XER_PRINT(NULL,&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);

    len = e2sm::encode(&asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,NULL,&m,&message);
    if (len < 0) {
	free(header);
	header = NULL;
	header_len = 0;
	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);
	return 1;
    }
    message_len = len;
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_E2SM_NEXRAN_E2SM_NexRAN_ControlMessage,&m);

    encoded = true;
    return true;
}

}
}

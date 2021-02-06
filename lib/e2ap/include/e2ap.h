#ifndef _E2AP_H_
#define _E2AP_H_

#include <list>
#include <cstdint>
#include <string>
#include <cstdlib>
#include <ctime>

#define E2AP_XER_PRINT(stream,type,pdu)					\
    do { if (e2ap::xer_print) xer_fprint(stream,type,pdu); } while (0);

namespace e2sm {

class Model;
class Control;
class Indication;

}

namespace e2ap {

extern bool xer_print;

/**
 * These are local function IDs.  Each service model might expose many
 * functions.  E2SM functions do not currently have global IDs, unless
 * you concat the E2SM OID and the function name.  There is no
 * requirement that function IDs be the same for different E2Setup/Reset
 * sessions, so we allow e2sm modules to register functions.
 */
typedef long RanFunctionId;

class RanFunction {
 public:
    RanFunctionId function_id;
    long revision;
    std::string name;
    std::string description;
    uint8_t *enc_definition;
    ssize_t enc_definition_len;
    int enabled;
    void *definition;
    e2sm::Model *model;
};

RanFunctionId get_next_ran_function_id();

typedef enum ControlRequestAck {
    CONTROL_REQUEST_NONE = 0,
    CONTROL_REQUEST_ACK,
    CONTROL_REQUEST_NACK
} ControlRequestAck_t;

class Message
{
 public:
    Message() : encoded(false),buf(NULL),len(0) {};
    virtual ~Message() {
	if (buf)
	    free(buf);
    }
    virtual bool encode() = 0;
    virtual unsigned char *get_buf() { return buf; };
    virtual ssize_t get_len() { return len; };

 protected:
    bool encoded;
    unsigned char *buf;
    ssize_t len;
};

class ControlRequest : public Message {
 public:
    ControlRequest(
        long requestor_id_,long instance_id_,RanFunctionId function_id_,
	e2sm::Control *model_,ControlRequestAck_t ack_request_)
	: requestor_id(requestor_id_),instance_id(instance_id_),
	  function_id(function_id_),model(model_),
	  ack_request(ack_request_),Message() {};
    virtual ~ControlRequest() = default;

    virtual void set_instance_id(long instance_id_) { instance_id = instance_id_; };

    virtual bool encode();

    long requestor_id;
    long instance_id;
    RanFunctionId function_id;
    e2sm::Control *model;
    ControlRequestAck_t ack_request;
};

class ControlAck {
 public:
    ControlRequest *request;
    long status;
    uint8_t outcome;
    size_t len;
};

class ControlFailure {
 public:
    ControlRequest *request;
    long cause;
    long cause_detail;
    uint8_t outcome;
    size_t len;
};

class Indication {
 public:
    long request_id;
    RanFunctionId function_id;
    long action_id;
    long serial_number;
    long type;
    e2sm::Indication *model;
    long call_process_id;
};

/**
 * These are generic service mechanisms.
 */
typedef enum {
    RIC_REPORT = 1,
    RIC_INSERT = 2,
    RIC_CONTROL = 3,
    RIC_POLICY = 4,
} service_t;

class RicInterface {
 public:
    virtual int handle_control_ack(ControlAck *control) = 0;
    virtual int handle_control_failure(ControlFailure *control) = 0;
    virtual int handle_indication(Indication *indication) = 0;
};

class E2AP {
 public:
    E2AP()
	: requestor_id(1),next_instance_id(1) {};
    // NB: current e2term does not like a random requestor_id; fails to
    // pass our message along.
    //{ std::srand(std::time(nullptr)); requestor_id = std::rand(); };
    virtual ~E2AP() = default;
    virtual bool init();

    long get_requestor_id() { return requestor_id; };
    long get_next_instance_id() { return next_instance_id++; };

 protected:
    long requestor_id;
    long next_instance_id;
    std::list<e2sm::Model *> models;
    RicInterface *ric_handler;
};

}

#endif /* _E2AP_H_ */

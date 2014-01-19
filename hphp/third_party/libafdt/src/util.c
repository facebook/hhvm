#include "config.h"
#include "afdt.h"

const struct afdt_error_t AFDT_ERROR_T_INIT = {
  .phase = AFDT_NO_PHASE,
  .operation = AFDT_NO_OPERATION,
  .message = "",
};


const char* afdt_phase_str(enum afdt_phase phase) {
  switch (phase) {
    case AFDT_NO_PHASE        : return "AFDT_NO_PHASE"        ;
    case AFDT_CREATE_SERVER   : return "AFDT_CREATE_SERVER"   ;
    case AFDT_ACCEPT_CLIENT   : return "AFDT_ACCEPT_CLIENT"   ;
    case AFDT_HANDLE_REQUEST  : return "AFDT_HANDLE_REQUEST"  ;
    case AFDT_CREATE_CLIENT   : return "AFDT_CREATE_CLIENT"   ;
    case AFDT_HANDLE_RESPONSE : return "AFDT_HANDLE_RESPONSE" ;
    default: return "UNKNOWN";
  }
}

const char* afdt_operation_str(enum afdt_operation operation) {
  switch (operation) {
    case AFDT_NO_OPERATION   : return "AFDT_NO_OPERATION"   ;
    case AFDT_MALLOC         : return "AFDT_MALLOC"         ;
    case AFDT_SOCKET         : return "AFDT_SOCKET"         ;
    case AFDT_PATHNAME       : return "AFDT_PATHNAME"       ;
    case AFDT_BIND           : return "AFDT_BIND"           ;
    case AFDT_LISTEN         : return "AFDT_LISTEN"         ;
    case AFDT_ACCEPT         : return "AFDT_ACCEPT"         ;
    case AFDT_CONNECT        : return "AFDT_CONNECT"        ;
    case AFDT_FORMAT         : return "AFDT_FORMAT"         ;
    case AFDT_SENDMSG        : return "AFDT_SENDMSG"        ;
    case AFDT_RECVMSG        : return "AFDT_RECVMSG"        ;
    case AFDT_EVENT_BASE_SET : return "AFDT_EVENT_BASE_SET" ;
    case AFDT_EVENT_ADD      : return "AFDT_EVENT_ADD"      ;
    case AFDT_POLL           : return "AFDT_POLL"           ;
    case AFDT_TIMEOUT        : return "AFDT_TIMEOUT"        ;
    default: return "UNKNOWN";
  }
}

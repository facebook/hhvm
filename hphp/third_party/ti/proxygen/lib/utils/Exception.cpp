#include "ti/proxygen/lib/utils/Exception.h"

namespace facebook { namespace proxygen {

Exception::Exception(std::string const &msg) :
    msg_(msg),
    code_(0) {
}

Exception::Exception(const Exception& exception) :
    msg_(exception.msg_),
    code_(exception.code_) {
}

Exception::Exception(Exception&& other) :
  msg_(other.msg_),
  code_(other.code_) {
}

Exception::~Exception(void) throw() {
}

const char *
Exception::what(void) const throw() {
  return msg_.c_str();
}

}}

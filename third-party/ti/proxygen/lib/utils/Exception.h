#pragma once
#include <string>
#include <utility>

#include "folly/Conv.h"

namespace facebook { namespace proxygen {

/**
 * Base class for all exceptions.
 */
class Exception : public std::exception {
 public:
  explicit Exception(std::string const &msg);
  Exception(const Exception& exception);
  Exception(Exception&&);

  template<typename... Args>
  explicit Exception(Args&&... args)
      : msg_(folly::to<std::string>(std::forward<Args>(args)...)),
        code_(0) {}

  virtual ~Exception(void) throw();

  // std::exception methods
  virtual const char *what(void) const throw();

  void setCode(int code) { code_ = code; }

  int getCode() const { return code_; }

 private:
  const std::string msg_;
  int code_;
};

}}


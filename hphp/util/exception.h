/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#ifndef incl_HPHP_EXCEPTION_H_
#define incl_HPHP_EXCEPTION_H_

#include <string>
#include <stdexcept>
#include <stdarg.h>

#include "hphp/util/portability.h"
#include "hphp/util/stack-trace.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Exception : std::exception {
  explicit Exception() = default;
  explicit Exception(const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
  explicit Exception(const std::string& msg);
  Exception(const Exception &e);

  // Try not to use this function (or the other varargs-based things) in new
  // code.  (You probably shouldn't be using Exception directly either.)
  void format(const char *fmt, va_list ap) ATTRIBUTE_PRINTF(2,0);

  void setMessage(const char *msg) { m_msg = msg ? msg : "";}

  ~Exception() noexcept override {}
  const char* what() const noexcept override;

  struct Deleter {
    Exception* m_e;
    Deleter() : m_e(nullptr) {}
    explicit Deleter(Exception* e) : m_e(e) {}
    ~Deleter() { delete m_e; }
  };

  virtual Exception* clone() {
    return new Exception(*this);
  }
  virtual void throwException() {
    Deleter deleter(this);
    throw *this;
  }

  /**
   * Error message without stacktrace.
   */
  const std::string &getMessage() const { return m_msg;}

protected:
  mutable std::string m_msg;
  mutable std::string m_what;
};

#define EXCEPTION_COMMON_IMPL(cls)               \
  cls* clone() override {                        \
    return new cls(*this);                       \
  }                                              \
  void throwException() override {               \
    Deleter deleter(this);                       \
    throw *this;                                 \
  }

///////////////////////////////////////////////////////////////////////////////

struct FileOpenException : Exception {
  explicit FileOpenException(const std::string& filename)
      : Exception("Unable to open file %s", filename.c_str()) {
  }

  EXCEPTION_COMMON_IMPL(FileOpenException);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXCEPTION_H_

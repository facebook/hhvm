/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/stack-trace.h"
#include <stdarg.h>

#include <string>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define EXCEPTION_COMMON_IMPL(cls) \
  virtual cls* clone() { \
    return new cls(*this); \
  } \
  virtual void throwException() { \
    Deleter deleter(this); \
    throw *this; \
  }

class Exception : public std::exception {
public:
  Exception(const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
  explicit Exception(const std::string& msg);
  Exception(const Exception &e);
  Exception();

  /**
   * Subclass can call this function to format variable length of parameters.
   *
   * class MyException : public Exception {
   * public:
   *   MyException(const char *fmt, ...) {
   *     va_list ap; va_start(ap, fmt); Format(fmt, ap); va_end(ap);
   *   }
   * };
   */
  void format(const char *fmt, va_list ap) ATTRIBUTE_PRINTF(2,0);

  void setMessage(const char *msg) { m_msg = msg ? msg : "";}

  virtual ~Exception() throw();
  virtual const char *what() const throw();

  struct Deleter {
    Exception* m_e;
    Deleter() : m_e(nullptr) {}
    explicit Deleter(Exception* e) : m_e(e) {}
    ~Deleter() { delete m_e; }
  };

  EXCEPTION_COMMON_IMPL(Exception);

  /**
   * Error message without stacktrace.
   */
  const std::string &getMessage() const { return m_msg;}

protected:
  mutable bool m_handled;
  mutable std::string m_msg;
  mutable std::string m_what;
};

///////////////////////////////////////////////////////////////////////////////

class FileOpenException : public Exception {
public:
  explicit FileOpenException(const char *filename)
      : Exception("Unable to open file %s", filename) {
  }

  EXCEPTION_COMMON_IMPL(FileOpenException);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXCEPTION_H_

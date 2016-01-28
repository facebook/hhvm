/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_CPP_BASE_EXCEPTIONS_H_
#define incl_HPHP_CPP_BASE_EXCEPTIONS_H_

#include <string>
#include <atomic>
#include <utility>

#include <folly/String.h>

#include "hphp/util/portability.h"
#include "hphp/util/exception.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/req-root.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct String;
struct IMarker;

//////////////////////////////////////////////////////////////////////

/*
 * ExtendedException is the exception type for C++ exceptions that carry PHP
 * stack traces, but do not represent user-visible PHP exception objects.
 *
 * This class should probably eventually be merged with FatalErrorException;
 * for now it's still here for historical reasons, though.
 *
 * You generally should not have to add new subclasses of these Exception types
 * in extension code---normally you want to go through the raise_error
 * machinery.
 */
struct ExtendedException : Exception {
  enum class SkipFrame {};

  explicit ExtendedException();
  explicit ExtendedException(const std::string& msg);
  explicit ExtendedException(SkipFrame frame, const std::string& msg);
  explicit ExtendedException(ATTRIBUTE_PRINTF_STRING const char* fmt, ...)
    ATTRIBUTE_PRINTF(2,3);
  ExtendedException(const ExtendedException& other);
  ExtendedException(ExtendedException&& other) noexcept;

  ExtendedException& operator=(const ExtendedException& other);
  ExtendedException& operator=(ExtendedException&& other) noexcept;

  EXCEPTION_COMMON_IMPL(ExtendedException);

  Array getBacktrace() const;
  void leakBacktrace() { m_btp.detach(); }
  std::pair<String,int> getFileAndLine() const;

  // a silent exception does not have its exception message logged
  bool isSilent() const { return m_silent; }
  void setSilent(bool s = true) { m_silent = s; }

protected:
  ExtendedException(const std::string& msg, ArrayData* backTrace);

private:
  void computeBacktrace(bool skipFrame = false);

private:
  req::root<Array> m_btp;
  bool m_silent{false};
};

struct FatalErrorException : ExtendedException {
  explicit FatalErrorException(const char *msg)
    : ExtendedException("%s", msg)
  {}
  FatalErrorException(int, ATTRIBUTE_PRINTF_STRING const char *msg, ...)
    ATTRIBUTE_PRINTF(3,4);
  FatalErrorException(const std::string&, const Array& backtrace,
                      bool isRecoverable = false);

  EXCEPTION_COMMON_IMPL(FatalErrorException);

  bool isRecoverable() const { return m_recoverable; }

private:
  bool m_recoverable{false};
};

ATTRIBUTE_NORETURN
void raise_fatal_error(const char* msg, const Array& bt = null_array,
                       bool recoverable = false, bool silent = false,
                       bool throws = true);

//////////////////////////////////////////////////////////////////////

struct ResourceExceededException : FatalErrorException {
  ResourceExceededException(const std::string& msg, const Array& backtrace)
    : FatalErrorException(msg, backtrace)
  {}
  EXCEPTION_COMMON_IMPL(ResourceExceededException);
};

struct RequestTimeoutException : ResourceExceededException {
  RequestTimeoutException(const std::string& msg, const Array& backtrace)
    : ResourceExceededException(msg, backtrace)
  {}
  EXCEPTION_COMMON_IMPL(RequestTimeoutException);
};

struct RequestCPUTimeoutException : ResourceExceededException {
  RequestCPUTimeoutException(const std::string& msg, const Array& backtrace)
    : ResourceExceededException(msg, backtrace)
  {}
  EXCEPTION_COMMON_IMPL(RequestCPUTimeoutException);
};

struct RequestMemoryExceededException : ResourceExceededException {
  RequestMemoryExceededException(const std::string& msg,
                                 const Array& backtrace)
    : ResourceExceededException(msg, backtrace)
  {}
  EXCEPTION_COMMON_IMPL(RequestMemoryExceededException);
};

//////////////////////////////////////////////////////////////////////

class ExitException : public ExtendedException {
public:
  static std::atomic<int> ExitCode; // XXX should not be static

  explicit ExitException(int exitCode) {
    ExitCode = exitCode;
  }
  EXCEPTION_COMMON_IMPL(ExitException);
};

class PhpFileDoesNotExistException : public ExtendedException {
public:
  explicit PhpFileDoesNotExistException(const char *file)
    : ExtendedException("File could not be loaded: %s", file) {}
  explicit PhpFileDoesNotExistException(const char *msg, bool empty_file)
    : ExtendedException("%s", msg) {
      assert(empty_file);
    }
  EXCEPTION_COMMON_IMPL(PhpFileDoesNotExistException);
};

//////////////////////////////////////////////////////////////////////

/*
 * These are deprecated functions for raising exceptions with certain error
 * messages.
 *
 * In newer code you'll generally want to use raise_error.
 */
ATTRIBUTE_NORETURN void throw_null_pointer_exception();
ATTRIBUTE_NORETURN void throw_invalid_object_type(const char* clsName);
ATTRIBUTE_NORETURN void throw_not_implemented(const char* feature);
ATTRIBUTE_NORETURN
void throw_not_supported(const char* feature, const char* reason);

//////////////////////////////////////////////////////////////////////

}

#endif

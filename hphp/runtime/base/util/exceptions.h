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

#ifndef incl_HPHP_CPP_BASE_EXCEPTIONS_H_
#define incl_HPHP_CPP_BASE_EXCEPTIONS_H_

#include <util/exception.h>
#include <util/util.h>
#include <runtime/base/types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// all defined exceptions

class ExtendedException : public Exception {
public:
  enum SkipFrame { skipFrame };
  ExtendedException();
  ExtendedException(const std::string &msg);
  ExtendedException(SkipFrame frame, const std::string &msg);
  ExtendedException(const char *fmt, ...);
  Array getBackTrace() const;
  virtual ~ExtendedException() throw() {}
  EXCEPTION_COMMON_IMPL(ExtendedException);
protected:
  ArrayHolder m_btp;
private:
  void computeBacktrace(bool skipFrame = false);
};

class Assertion : public ExtendedException {
public:
  Assertion() : ExtendedException("An assertion was raised.") {}
  virtual ~Assertion() throw() {}
  EXCEPTION_COMMON_IMPL(Assertion);
};

class NullPointerException : public ExtendedException {
public:
  NullPointerException()
    : ExtendedException("A null object pointer was used.") {}
  virtual ~NullPointerException() throw() {}
  EXCEPTION_COMMON_IMPL(NullPointerException);
};

class InvalidObjectTypeException : public ExtendedException {
public:
  InvalidObjectTypeException(const char *name)
    : ExtendedException("Unexpected object type %s.", name) {}
  virtual ~InvalidObjectTypeException() throw() {}
  EXCEPTION_COMMON_IMPL(InvalidObjectTypeException);
};

class InvalidOperandException : public ExtendedException {
public:
  InvalidOperandException(const char *msg)
    : ExtendedException("Invalid operand type was used: %s.", msg) {}
  virtual ~InvalidOperandException() throw() {}
  EXCEPTION_COMMON_IMPL(InvalidOperandException);
};

class BadArrayMergeException : public InvalidOperandException {
public:
  BadArrayMergeException()
    : InvalidOperandException("merging an array with NULL or non-array") {}
  virtual ~BadArrayMergeException() throw() {}
  EXCEPTION_COMMON_IMPL(BadArrayMergeException);
};

class BadArrayOperandException : public InvalidOperandException {
public:
  BadArrayOperandException()
    : InvalidOperandException("cannot perform this operation with arrays") {}
  virtual ~BadArrayOperandException() throw() {}
  EXCEPTION_COMMON_IMPL(BadArrayOperandException);
};

class BadTypeConversionException : public ExtendedException {
public:
  BadTypeConversionException(const char *msg)
    : ExtendedException("Bad type conversion: %s.", msg) {}
  virtual ~BadTypeConversionException() throw() {}
  EXCEPTION_COMMON_IMPL(BadTypeConversionException);
};

class OffsetOutOfRangeException : public ExtendedException {
public:
  OffsetOutOfRangeException()
    : ExtendedException("String offset is out of range.") {}
  virtual ~OffsetOutOfRangeException() throw() {}
  EXCEPTION_COMMON_IMPL(OffsetOutOfRangeException);
};

class EmptyObjectPropertyException : public ExtendedException {
public:
  EmptyObjectPropertyException()
    : ExtendedException("Cannot access empty property") {}
  virtual ~EmptyObjectPropertyException() throw() {}
  EXCEPTION_COMMON_IMPL(EmptyObjectPropertyException);
};

class NullStartObjectPropertyException : public ExtendedException {
public:
  NullStartObjectPropertyException()
    : ExtendedException("Cannot access property started with '\\0'") {}
  virtual ~NullStartObjectPropertyException() throw() {}
  EXCEPTION_COMMON_IMPL(NullStartObjectPropertyException);
};

class InvalidFunctionCallException : public ExtendedException {
public:
  InvalidFunctionCallException(const char *func)
    : ExtendedException("(1) call the function without enough arguments OR "
                        "(2) Unable to find function \"%s\" OR "
                        "(3) function was not in invoke table OR "
                        "(4) function was renamed to something else.",
                        func) {}
  virtual ~InvalidFunctionCallException() throw() {}
  EXCEPTION_COMMON_IMPL(InvalidFunctionCallException);
};

class InvalidClassException : public ExtendedException {
public:
  InvalidClassException(const char *cls)
    : ExtendedException("Unable to find class \"%s\".", cls) {}
  virtual ~InvalidClassException() throw() {}
  EXCEPTION_COMMON_IMPL(InvalidClassException);
};

class ParseTimeFatalException : public Exception {
public:
  ParseTimeFatalException(const char* file, int line,
                          const char* msg, ...)
      : m_file(file), m_line(line) {
    va_list ap; va_start(ap, msg); format(msg, ap); va_end(ap);
  }
  virtual ~ParseTimeFatalException() throw() {}
  EXCEPTION_COMMON_IMPL(ParseTimeFatalException);

  std::string m_file;
  int m_line;
};

class FatalErrorException : public ExtendedException {
public:
  FatalErrorException(const char *msg) : ExtendedException("%s", msg) {}
  FatalErrorException(int, const char *msg, ...) {
    va_list ap; va_start(ap, msg); format(msg, ap); va_end(ap);
  }
  FatalErrorException(const std::string &msg, const Array& backtrace);
  virtual ~FatalErrorException() throw() {}
  EXCEPTION_COMMON_IMPL(FatalErrorException);
};

class UncatchableException : public ExtendedException {
public:
  UncatchableException(const char *msg) : ExtendedException(msg) {}
  virtual ~UncatchableException() throw() {}
  EXCEPTION_COMMON_IMPL(UncatchableException);
};

class ClassNotFoundException : public FatalErrorException {
public:
  ClassNotFoundException(const char *msg)
    : FatalErrorException(msg) {}
  virtual ~ClassNotFoundException() throw() {}
  EXCEPTION_COMMON_IMPL(ClassNotFoundException);
};

class SystemCallFailure : public ExtendedException {
public:
  SystemCallFailure(const char *func)
    : ExtendedException("%s returned %d: %s.", func, errno,
                        Util::safe_strerror(errno).c_str()) {}
  virtual ~SystemCallFailure() throw() {}
  EXCEPTION_COMMON_IMPL(SystemCallFailure);
};

class InvalidArgumentException : public ExtendedException {
public:
  InvalidArgumentException(const char *param, int value)
    : ExtendedException("Invalid argument \"%s\": [%d]", param, value) {}

  InvalidArgumentException(const char *param, unsigned int value)
    : ExtendedException("Invalid argument \"%s\": [%u]", param, value) {}

  InvalidArgumentException(const char *param, long value)
    : ExtendedException("Invalid argument \"%s\": [%ld]", param, value) {}

  InvalidArgumentException(const char *param, long long value)
    : ExtendedException("Invalid argument \"%s\": [%lld]", param, value) {}

  InvalidArgumentException(const char *param, double value)
    : ExtendedException("Invalid argument \"%s\": [%g]", param, value) {}

  InvalidArgumentException(const char *param, const char *value)
    : ExtendedException("Invalid argument \"%s\": [%s]", param, value) {}

  InvalidArgumentException(const char *param, const std::string &value)
    : ExtendedException("Invalid argument \"%s\": [%s]", param,
                        value.c_str()) {}

  InvalidArgumentException(int, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt); format(fmt, ap); va_end(ap);
  }

  InvalidArgumentException(const char *param)
    : ExtendedException("Invalid argument: %s", param) {}

  virtual ~InvalidArgumentException() throw() {}
  EXCEPTION_COMMON_IMPL(InvalidArgumentException);
};

class NotEnoughArgumentsException : public ExtendedException {
public:
  NotEnoughArgumentsException(const char *funcname)
    : ExtendedException("Not enough arguments for function %s", funcname) {}
  virtual ~NotEnoughArgumentsException() throw() {}
  EXCEPTION_COMMON_IMPL(NotEnoughArgumentsException);
};

class TooManyArgumentsException : public ExtendedException {
public:
  TooManyArgumentsException(const char *funcname)
    : ExtendedException("Too much arguments for function %s", funcname) {}
  virtual ~TooManyArgumentsException() throw() {}
  EXCEPTION_COMMON_IMPL(TooManyArgumentsException);
};

class TypeVariableChangeException : public ExtendedException {
public:
  TypeVariableChangeException(const char *loc)
    : ExtendedException("Type of variable changed at %s", loc) {}
  virtual ~TypeVariableChangeException() throw() {}
  EXCEPTION_COMMON_IMPL(TypeVariableChangeException);
};

class UseOfUndefinedVarException : public ExtendedException {
public:
  UseOfUndefinedVarException(const char *loc)
    : ExtendedException("Use of undefined variable at %s", loc) {}
  virtual ~UseOfUndefinedVarException() throw() {}
  EXCEPTION_COMMON_IMPL(UseOfUndefinedVarException);
};

class MethodSignatureChangeException : public ExtendedException {
public:
  MethodSignatureChangeException(const char *method)
    : ExtendedException("Signature of method %s changed", method) {}
  virtual ~MethodSignatureChangeException() throw() {}
  EXCEPTION_COMMON_IMPL(MethodSignatureChangeException);
};

class NestingLevelTooDeepException : public ExtendedException {
public:
  NestingLevelTooDeepException()
    : ExtendedException("Nesting level too deep - recursive dependency?") {}
  virtual ~NestingLevelTooDeepException() throw() {}
  EXCEPTION_COMMON_IMPL(NestingLevelTooDeepException);
};

class NotImplementedException : public ExtendedException {
public:
  NotImplementedException(const char *feature)
    : ExtendedException("%s is not implemented yet.", feature) {}
  virtual ~NotImplementedException() throw() {}
  EXCEPTION_COMMON_IMPL(NotImplementedException);
};

class NotSupportedException : public ExtendedException {
public:
  NotSupportedException(const char *feature, const char *reason)
    : ExtendedException("%s is not going to be supported: %s",
                        feature, reason) {}
  virtual ~NotSupportedException() throw() {}
  EXCEPTION_COMMON_IMPL(NotSupportedException);
};

class ExitException : public ExtendedException {
public:
  static int ExitCode;

  ExitException(int exitCode) {
    m_handled = false;
    ExitCode = exitCode;
  }
  virtual ~ExitException() throw() {}
  EXCEPTION_COMMON_IMPL(ExitException);
};

class PhpFileDoesNotExistException : public ExtendedException {
public:
  PhpFileDoesNotExistException(const char *file)
    : ExtendedException("File could not be loaded: %s", file) {}
  virtual ~PhpFileDoesNotExistException() throw() {}
  EXCEPTION_COMMON_IMPL(PhpFileDoesNotExistException);
};

void throw_null_pointer_exception() ATTRIBUTE_COLD ATTRIBUTE_NORETURN;

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_CPP_BASE_EXCEPTIONS_H_

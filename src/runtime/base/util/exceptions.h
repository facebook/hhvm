/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __CPP_BASE_EXCEPTIONS_H__
#define __CPP_BASE_EXCEPTIONS_H__

#include <util/exception.h>
#include <util/util.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// all defined exceptions

DECLARE_BOOST_TYPES(Array);
class ExtendedException : public Exception {
public:
  ExtendedException();
  ExtendedException(const std::string &msg);
  ExtendedException(const char *fmt, ...);
  ArrayPtr getBackTrace() const { return m_bt; }
  virtual ~ExtendedException() throw() {}
protected:
  ArrayPtr m_bt;
};

class Assertion : public ExtendedException {
public:
  Assertion() : ExtendedException("An assertion was raised.") {}
  virtual ~Assertion() throw() {}
};

class NullPointerException : public ExtendedException {
public:
  NullPointerException()
    : ExtendedException("A null object pointer was used.") {}
  virtual ~NullPointerException() throw() {}
};

class InvalidObjectTypeException : public ExtendedException {
public:
  InvalidObjectTypeException(const char *name)
    : ExtendedException("Unexpected object type %s.", name) {}
  virtual ~InvalidObjectTypeException() throw() {}
};

class InvalidOperandException : public ExtendedException {
public:
  InvalidOperandException(const char *msg)
    : ExtendedException("Invalid operand type was used: %s.", msg) {}
  virtual ~InvalidOperandException() throw() {}
};

class BadArrayMergeException : public InvalidOperandException {
public:
  BadArrayMergeException()
    : InvalidOperandException("merging an array with NULL or non-array") {}
  virtual ~BadArrayMergeException() throw() {}
};

class BadArrayOperandException : public InvalidOperandException {
public:
  BadArrayOperandException()
    : InvalidOperandException("cannot perform this operation with arrays") {}
  virtual ~BadArrayOperandException() throw() {}
};

class BadTypeConversionException : public ExtendedException {
public:
  BadTypeConversionException(const char *msg)
    : ExtendedException("Bad type conversion: %s.", msg) {}
  virtual ~BadTypeConversionException() throw() {}
};

class OffsetOutOfRangeException : public ExtendedException {
public:
  OffsetOutOfRangeException()
    : ExtendedException("String offset is out of range.") {}
  virtual ~OffsetOutOfRangeException() throw() {}
};

class EmptyObjectPropertyException : public ExtendedException {
public:
  EmptyObjectPropertyException()
    : ExtendedException("Object property name cannot be empty.") {}
  virtual ~EmptyObjectPropertyException() throw() {}
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
};

class InvalidClassException : public ExtendedException {
public:
  InvalidClassException(const char *cls)
    : ExtendedException("Unable to find class \"%s\".", cls) {}
  virtual ~InvalidClassException() throw() {}
};

class FatalErrorException : public ExtendedException {
public:
  FatalErrorException(const char *msg) : ExtendedException("%s", msg) {}
  FatalErrorException(int, const char *msg, ...) {
    va_list ap; va_start(ap, msg); format(msg, ap); va_end(ap);
  }
  FatalErrorException(const std::string &msg, ArrayPtr backtrace);
  virtual ~FatalErrorException() throw() {}
};

class UncatchableException : public ExtendedException {
public:
  UncatchableException(const char *msg) : ExtendedException(msg) {}
  virtual ~UncatchableException() throw() {}
};

class ClassNotFoundException : public FatalErrorException {
public:
  ClassNotFoundException(const char *msg)
    : FatalErrorException(msg) {}
  virtual ~ClassNotFoundException() throw() {}
};

class SystemCallFailure : public ExtendedException {
public:
  SystemCallFailure(const char *func)
    : ExtendedException("%s returned %d: %s.", func, errno,
                        Util::safe_strerror(errno).c_str()) {}
  virtual ~SystemCallFailure() throw() {}
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
};

class NotEnoughArgumentsException : public ExtendedException {
public:
  NotEnoughArgumentsException(const char *funcname)
    : ExtendedException("Not enough arguments for function %s", funcname) {}
  virtual ~NotEnoughArgumentsException() throw() {}
};

class TooManyArgumentsException : public ExtendedException {
public:
  TooManyArgumentsException(const char *funcname)
    : ExtendedException("Too much arguments for function %s", funcname) {}
  virtual ~TooManyArgumentsException() throw() {}
};

class TypeVariableChangeException : public ExtendedException {
public:
  TypeVariableChangeException(const char *loc)
    : ExtendedException("Type of variable changed at %s", loc) {}
  virtual ~TypeVariableChangeException() throw() {}
};

class UseOfUndefinedVarException : public ExtendedException {
public:
  UseOfUndefinedVarException(const char *loc)
    : ExtendedException("Use of undefined variable at %s", loc) {}
  virtual ~UseOfUndefinedVarException() throw() {}
};

class MethodSignatureChangeException : public ExtendedException {
public:
  MethodSignatureChangeException(const char *method)
    : ExtendedException("Signature of method %s changed", method) {}
  virtual ~MethodSignatureChangeException() throw() {}
};

class NestingLevelTooDeepException : public ExtendedException {
public:
  NestingLevelTooDeepException()
    : ExtendedException("Nesting level too deep - recursive dependency?") {}
  virtual ~NestingLevelTooDeepException() throw() {}
};

class NotImplementedException : public ExtendedException {
public:
  NotImplementedException(const char *feature)
    : ExtendedException("%s is not implemented yet.", feature) {}
  virtual ~NotImplementedException() throw() {}
};

class NotSupportedException : public ExtendedException {
public:
  NotSupportedException(const char *feature, const char *reason)
    : ExtendedException("%s is not going to be supported: %s",
                        feature, reason) {}
  virtual ~NotSupportedException() throw() {}
};

class ExitException : public ExtendedException {
public:
  static int ExitCode;

  ExitException(int exitCode) {
    m_handled = false;
    ExitCode = exitCode;
  }
  virtual ~ExitException() throw() {}
};

class PhpFileDoesNotExistException : public ExtendedException {
public:
  PhpFileDoesNotExistException(const char *file)
    : ExtendedException("File could not be loaded: %s", file) {}
  virtual ~PhpFileDoesNotExistException() throw() {}
};

void throw_null_pointer_exception() ATTRIBUTE_COLD __attribute__((noreturn));

///////////////////////////////////////////////////////////////////////////////
}

#endif // __CPP_BASE_EXCEPTIONS_H__

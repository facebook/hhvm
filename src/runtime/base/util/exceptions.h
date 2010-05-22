/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
private:
  ArrayPtr m_bt;
};

class Assertion : public ExtendedException {
public:
  Assertion() : ExtendedException("An assertion was raised.") {}
};

class NullPointerException : public ExtendedException {
public:
  NullPointerException()
    : ExtendedException("A null object pointer was used.") {}
};

class InvalidObjectTypeException : public ExtendedException {
public:
  InvalidObjectTypeException(const char *name)
    : ExtendedException("Unexpected object type %s.", name) {}
};

class InvalidOperandException : public ExtendedException {
public:
  InvalidOperandException(const char *msg)
    : ExtendedException("Invalid operand type was used: %s.", msg) {}
};

class BadArrayMergeException : public InvalidOperandException {
public:
  BadArrayMergeException()
    : InvalidOperandException("merging an array with NULL or non-array") {}
};

class BadArrayOperandException : public InvalidOperandException {
public:
  BadArrayOperandException()
    : InvalidOperandException("cannot perform this operation with arrays") {}
};

class BadTypeConversionException : public ExtendedException {
public:
  BadTypeConversionException(const char *msg)
    : ExtendedException("Bad type conversion: %s.", msg) {}
};

class OffsetOutOfRangeException : public ExtendedException {
public:
  OffsetOutOfRangeException()
    : ExtendedException("String offset is out of range.") {}
};

class EmptyObjectPropertyException : public ExtendedException {
public:
  EmptyObjectPropertyException()
    : ExtendedException("Object property name cannot be empty.") {}
};

class InvalidFunctionCallException : public ExtendedException {
public:
  InvalidFunctionCallException(const char *func)
    : ExtendedException("(1) call the function without enough arguments OR "
                        "(2) Unable to find function \"%s\" OR "
                        "(3) function was not in invoke table.",
                        func) {}
};

class InvalidClassException : public ExtendedException {
public:
  InvalidClassException(const char *cls)
    : ExtendedException("Unable to find class \"%s\".", cls) {}
};

class FatalErrorException : public ExtendedException {
public:
  FatalErrorException(const char *msg, ...) {
    va_list ap; va_start(ap, msg); format(msg, ap); va_end(ap);
  }
};

class UncatchableException : public ExtendedException {
public:
  UncatchableException(const char *msg) : ExtendedException(msg) {}
};

class ClassNotFoundException : public FatalErrorException {
public:
  ClassNotFoundException(const char *msg)
    : FatalErrorException(msg) {}
};

class SystemCallFailure : public ExtendedException {
public:
  SystemCallFailure(const char *func)
    : ExtendedException("%s returned %d: %s.", func, errno,
                        Util::safe_strerror(errno).c_str()) {}
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
};

class NotEnoughArgumentsException : public ExtendedException {
public:
  NotEnoughArgumentsException(const char *funcname)
    : ExtendedException("Not enough arguments for function %s", funcname) {}
};

class TooManyArgumentsException : public ExtendedException {
public:
  TooManyArgumentsException(const char *funcname)
    : ExtendedException("Too much arguments for function %s", funcname) {}
};

class TypeVariableChangeException : public ExtendedException {
public:
  TypeVariableChangeException(const char *loc)
    : ExtendedException("Type of variable changed at %s", loc) {}
};

class UseOfUndefinedVarException : public ExtendedException {
public:
  UseOfUndefinedVarException(const char *loc)
    : ExtendedException("Use of undefined variable at %s", loc) {}
};

class MethodSignatureChangeException : public ExtendedException {
public:
  MethodSignatureChangeException(const char *method)
    : ExtendedException("Signature of method %s changed", method) {}
};

class NestingLevelTooDeepException : public ExtendedException {
public:
  NestingLevelTooDeepException()
    : ExtendedException("Nesting level too deep - recursive dependency?") {}
};

class NotImplementedException : public ExtendedException {
public:
  NotImplementedException(const char *feature)
    : ExtendedException("%s is not implemented yet.", feature) {}
};

class NotSupportedException : public ExtendedException {
public:
  NotSupportedException(const char *feature, const char *reason)
    : ExtendedException("%s is not going to be supported: %s",
                        feature, reason) {}
};

class ExitException : public Exception {
public:
  static int ExitCode;

  ExitException(int exitCode) : Exception(false) {
    ExitCode = exitCode;
  }
};

class PhpFileDoesNotExistException : public ExtendedException {
public:
  PhpFileDoesNotExistException(const char *file)
    : ExtendedException("File could not be loaded: %s", file) {}
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __CPP_BASE_EXCEPTIONS_H__

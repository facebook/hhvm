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
  virtual ExtendedException* clone() { return new ExtendedException(*this); }
  virtual void throwException() { throw *this; }
protected:
  ArrayPtr m_bt;
};

class Assertion : public ExtendedException {
public:
  Assertion() : ExtendedException("An assertion was raised.") {}
  virtual ~Assertion() throw() {}
  virtual Assertion* clone() { return new Assertion(*this); }
  virtual void throwException() { throw *this; }
};

class NullPointerException : public ExtendedException {
public:
  NullPointerException()
    : ExtendedException("A null object pointer was used.") {}
  virtual ~NullPointerException() throw() {}
  virtual NullPointerException* clone() {
    return new NullPointerException(*this);
  }
  virtual void throwException() { throw *this; }
};

class InvalidObjectTypeException : public ExtendedException {
public:
  InvalidObjectTypeException(const char *name)
    : ExtendedException("Unexpected object type %s.", name) {}
  virtual ~InvalidObjectTypeException() throw() {}
  virtual InvalidObjectTypeException* clone() {
    return new InvalidObjectTypeException(*this);
  }
  virtual void throwException() { throw *this; }
};

class InvalidOperandException : public ExtendedException {
public:
  InvalidOperandException(const char *msg)
    : ExtendedException("Invalid operand type was used: %s.", msg) {}
  virtual ~InvalidOperandException() throw() {}
  virtual InvalidOperandException* clone() {
    return new InvalidOperandException(*this);
  }
  virtual void throwException() { throw *this; }
};

class BadArrayMergeException : public InvalidOperandException {
public:
  BadArrayMergeException()
    : InvalidOperandException("merging an array with NULL or non-array") {}
  virtual ~BadArrayMergeException() throw() {}
  virtual BadArrayMergeException* clone() {
    return new BadArrayMergeException(*this);
  }
  virtual void throwException() { throw *this; }
};

class BadArrayOperandException : public InvalidOperandException {
public:
  BadArrayOperandException()
    : InvalidOperandException("cannot perform this operation with arrays") {}
  virtual ~BadArrayOperandException() throw() {}
  virtual BadArrayOperandException* clone() {
    return new BadArrayOperandException(*this);
  }
  virtual void throwException() { throw *this; }
};

class BadTypeConversionException : public ExtendedException {
public:
  BadTypeConversionException(const char *msg)
    : ExtendedException("Bad type conversion: %s.", msg) {}
  virtual ~BadTypeConversionException() throw() {}
  virtual BadTypeConversionException* clone() {
    return new BadTypeConversionException(*this);
  }
  virtual void throwException() { throw *this; }
};

class OffsetOutOfRangeException : public ExtendedException {
public:
  OffsetOutOfRangeException()
    : ExtendedException("String offset is out of range.") {}
  virtual ~OffsetOutOfRangeException() throw() {}
  virtual OffsetOutOfRangeException* clone() {
    return new OffsetOutOfRangeException(*this);
  }
  virtual void throwException() { throw *this; }
};

class EmptyObjectPropertyException : public ExtendedException {
public:
  EmptyObjectPropertyException()
    : ExtendedException("Object property name cannot be empty.") {}
  virtual ~EmptyObjectPropertyException() throw() {}
  virtual EmptyObjectPropertyException* clone() {
    return new EmptyObjectPropertyException(*this);
  }
  virtual void throwException() { throw *this; }
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
  virtual InvalidFunctionCallException* clone() {
    return new InvalidFunctionCallException(*this);
  }
  virtual void throwException() { throw *this; }
};

class InvalidClassException : public ExtendedException {
public:
  InvalidClassException(const char *cls)
    : ExtendedException("Unable to find class \"%s\".", cls) {}
  virtual ~InvalidClassException() throw() {}
  virtual InvalidClassException* clone() {
    return new InvalidClassException(*this);
  }
  virtual void throwException() { throw *this; }
};

class FatalErrorException : public ExtendedException {
public:
  FatalErrorException(const char *msg) : ExtendedException("%s", msg) {}
  FatalErrorException(int, const char *msg, ...) {
    va_list ap; va_start(ap, msg); format(msg, ap); va_end(ap);
  }
  FatalErrorException(const std::string &msg, ArrayPtr backtrace);
  virtual ~FatalErrorException() throw() {}
  virtual FatalErrorException* clone() {
    return new FatalErrorException(*this);
  }
  virtual void throwException() { throw *this; }
};

class UncatchableException : public ExtendedException {
public:
  UncatchableException(const char *msg) : ExtendedException(msg) {}
  virtual ~UncatchableException() throw() {}
  virtual UncatchableException* clone() {
    return new UncatchableException(*this);
  }
  virtual void throwException() { throw *this; }
};

class ClassNotFoundException : public FatalErrorException {
public:
  ClassNotFoundException(const char *msg)
    : FatalErrorException(msg) {}
  virtual ~ClassNotFoundException() throw() {}
  virtual ClassNotFoundException* clone() {
    return new ClassNotFoundException(*this);
  }
  virtual void throwException() { throw *this; }
};

class SystemCallFailure : public ExtendedException {
public:
  SystemCallFailure(const char *func)
    : ExtendedException("%s returned %d: %s.", func, errno,
                        Util::safe_strerror(errno).c_str()) {}
  virtual ~SystemCallFailure() throw() {}
  virtual SystemCallFailure* clone() {
    return new SystemCallFailure(*this);
  }
  virtual void throwException() { throw *this; }
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
  virtual InvalidArgumentException* clone() {
    return new InvalidArgumentException(*this);
  }
  virtual void throwException() { throw *this; }
};

class NotEnoughArgumentsException : public ExtendedException {
public:
  NotEnoughArgumentsException(const char *funcname)
    : ExtendedException("Not enough arguments for function %s", funcname) {}
  virtual ~NotEnoughArgumentsException() throw() {}
  virtual NotEnoughArgumentsException* clone() {
    return new NotEnoughArgumentsException(*this);
  }
  virtual void throwException() { throw *this; }
};

class TooManyArgumentsException : public ExtendedException {
public:
  TooManyArgumentsException(const char *funcname)
    : ExtendedException("Too much arguments for function %s", funcname) {}
  virtual ~TooManyArgumentsException() throw() {}
  virtual TooManyArgumentsException* clone() {
    return new TooManyArgumentsException(*this);
  }
  virtual void throwException() { throw *this; }
};

class TypeVariableChangeException : public ExtendedException {
public:
  TypeVariableChangeException(const char *loc)
    : ExtendedException("Type of variable changed at %s", loc) {}
  virtual ~TypeVariableChangeException() throw() {}
  virtual TypeVariableChangeException* clone() {
    return new TypeVariableChangeException(*this);
  }
  virtual void throwException() { throw *this; }
};

class UseOfUndefinedVarException : public ExtendedException {
public:
  UseOfUndefinedVarException(const char *loc)
    : ExtendedException("Use of undefined variable at %s", loc) {}
  virtual ~UseOfUndefinedVarException() throw() {}
  virtual UseOfUndefinedVarException* clone() {
    return new UseOfUndefinedVarException(*this);
  }
  virtual void throwException() { throw *this; }
};

class MethodSignatureChangeException : public ExtendedException {
public:
  MethodSignatureChangeException(const char *method)
    : ExtendedException("Signature of method %s changed", method) {}
  virtual ~MethodSignatureChangeException() throw() {}
  virtual MethodSignatureChangeException* clone() {
    return new MethodSignatureChangeException(*this);
  }
  virtual void throwException() { throw *this; }
};

class NestingLevelTooDeepException : public ExtendedException {
public:
  NestingLevelTooDeepException()
    : ExtendedException("Nesting level too deep - recursive dependency?") {}
  virtual ~NestingLevelTooDeepException() throw() {}
  virtual NestingLevelTooDeepException* clone() {
    return new NestingLevelTooDeepException(*this);
  }
  virtual void throwException() { throw *this; }
};

class NotImplementedException : public ExtendedException {
public:
  NotImplementedException(const char *feature)
    : ExtendedException("%s is not implemented yet.", feature) {}
  virtual ~NotImplementedException() throw() {}
  virtual NotImplementedException* clone() {
    return new NotImplementedException(*this);
  }
  virtual void throwException() { throw *this; }
};

class NotSupportedException : public ExtendedException {
public:
  NotSupportedException(const char *feature, const char *reason)
    : ExtendedException("%s is not going to be supported: %s",
                        feature, reason) {}
  virtual ~NotSupportedException() throw() {}
  virtual NotSupportedException* clone() {
    return new NotSupportedException(*this);
  }
  virtual void throwException() { throw *this; }
};

class ExitException : public ExtendedException {
public:
  static int ExitCode;

  ExitException(int exitCode) {
    m_handled = false;
    ExitCode = exitCode;
  }
  virtual ~ExitException() throw() {}
  virtual ExitException* clone() {
    return new ExitException(*this);
  }
  virtual void throwException() { throw *this; }
};

class PhpFileDoesNotExistException : public ExtendedException {
public:
  PhpFileDoesNotExistException(const char *file)
    : ExtendedException("File could not be loaded: %s", file) {}
  virtual ~PhpFileDoesNotExistException() throw() {}
  virtual PhpFileDoesNotExistException* clone() {
    return new PhpFileDoesNotExistException(*this);
  }
  virtual void throwException() { throw *this; }
};

void throw_null_pointer_exception() ATTRIBUTE_COLD __attribute__((noreturn));

///////////////////////////////////////////////////////////////////////////////
}

#endif // __CPP_BASE_EXCEPTIONS_H__

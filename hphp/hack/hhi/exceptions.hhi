<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/**
 * This file provides type information for some of PHP's predefined classes
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

namespace {

<<__Sealed(Error::class, Exception::class)>>
interface Throwable {
  public function getMessage(): string;
  // Documented as 'int' in PHP docs, but not actually guaranteed;
  // subclasses (e.g. PDO) can do what they want.
  public function getCode()[]: mixed;
  public function getFile()[]: string;
  public function getLine()[]: int;
  public function getTrace()[]: Container<mixed>;
  public function getTraceAsString()[]: string;
  public function getPrevious()[]: ?Throwable;
  public function __toString(): string;
  public function toString(): string;
}

class Error implements Throwable {
  protected string $message;
  protected mixed $code;
  protected string $file;
  protected int $line;

  /* Methods */
  public function __construct (
    string $message = "",
    int $code = 0,
    ?Throwable $previous = null,
  )[];
  final public function getMessage()[]: string;
  final public function getPrevious()[]: ?Throwable;
  final public function getCode()[]: mixed;
  final public function getFile()[]: string;
  final public function getLine()[]: int;
  final public function getTrace()[]: varray<mixed>;
  final public function getTraceAsString()[]: string;
  public function __toString(): string;
  public function toString(): string;
  final private function __clone(): void;
}

class ArithmeticError extends Error {}
class ArgumentCountError extends Error {}
class AssertionError extends Error {}
class DivisionByZeroError extends Error {}
class ParseError extends Error {}
class TypeError extends Error {}

interface IExceptionWithPureGetMessage {
  require extends Exception;

  public function getMessage()[]: string;
}

class ExceptionWithPureGetMessage
  extends Exception implements IExceptionWithPureGetMessage {

  public function getMessage()[]: string;
}

class Exception implements Throwable {
  protected int $code;
  protected string $file;
  protected int $line;
  private varray<mixed> $trace;

  public function __construct (
    protected string $message = '',
    int $code = 0,
    protected ?Exception $previous = null,
  )[];

  // TODO(coeffects) How do we fix this?
  public function getMessage(): string;
  final public function getPrevious()[]: ?Exception;
  public final function setPrevious(Exception $previous)[write_props]: void;
  public function getCode()[]: int;
  final public function getFile()[]: string;
  final public function getLine()[]: int;
  final public function getTrace()[]: varray<mixed>;
  final protected function __prependTrace(Container<mixed> $trace)[write_props]: void;
  final public function getTraceAsString()[]: string;
  public function __toString(): string;
  public function toString(): string;
  final private function __clone(): void;

  final public static function getTraceOptions()[read_globals];
  final public static function setTraceOptions($opts)[globals];
}

class ErrorException extends ExceptionWithPureGetMessage {
  public function __construct(
    $message = "",
    int $code = 0,
    protected int $severity = 0,
    string $filename = '' /* __FILE__ */,
    int $lineno = 0 /* __LINE__ */,
    ?Exception $previous = null
  )[];
  public final function getSeverity()[]: int;
}

class LogicException extends ExceptionWithPureGetMessage {}
class BadFunctionCallException extends LogicException {}
class BadMethodCallException extends BadFunctionCallException {}
class DomainException extends LogicException {}
class InvalidArgumentException extends LogicException {}
class LengthException extends LogicException {}
class OutOfRangeException extends LogicException {}
final class InvalidCallbackArgumentException extends LogicException {}
final class InvalidForeachArgumentException extends LogicException {}
final class TypecastException extends LogicException {}
final class UndefinedPropertyException extends LogicException {}
final class UndefinedVariableException extends LogicException {}
final class AccessPropertyOnNonObjectException extends LogicException {}

class RuntimeException extends ExceptionWithPureGetMessage {}
class OutOfBoundsException extends RuntimeException {}
class OverflowException extends RuntimeException {}
class RangeException extends RuntimeException {}
class UnderflowException extends RuntimeException {}
class UnexpectedValueException extends RuntimeException {}

class InvariantException extends Exception {}
final class TypeAssertionException extends RuntimeException {}
class DivisionByZeroException extends ExceptionWithPureGetMessage {}

} // namespace

namespace HH {

class InvariantException extends \Exception {}

} // namespace HH

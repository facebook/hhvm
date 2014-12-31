<?hh // decl /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/**
 * This file provides type information for some of PHP's predefined classes
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */

class Exception {
  // $code should be untyped, or mixed because some subclasses set it
  // to a string, the main example being PDOException
  protected $code;
  protected string $file;
  protected int $line;
  protected array $trace;

  public function __construct (
    protected string $message = '',
    int $code = 0,
    protected ?Exception $previous = null,
  );
  public function getMessage(): string;
  final public function getPrevious(): Exception;
  public final function setPrevious(Exception $previous): void;
  public function getCode(): int;
  final public function getFile(): string;
  final public function getLine(): int;
  final public function getTrace(): array;
  final public function getTraceAsString(): string;
  public function __toString(): string;
  final private function __clone(): void;

  public static function getTraceOptions();
  public static function setTraceOptions($opts);
}

class ErrorException extends Exception {
  public function __construct(
    $message = "",
    int $code = 0,
    protected int $severity = 0,
    string $filename = '' /* __FILE__ */,
    int $lineno = 0 /* __LINE__ */,
    ?Exception $previous = null
  );
  public final function getSeverity(): int;
}

class LogicException extends Exception {}
class BadFunctionCallException extends LogicException {}
class BadMethodCallException extends BadFunctionCallException {}
class DomainException extends LogicException {}
class InvalidArgumentException extends LogicException {}
class LengthException extends LogicException {}
class OutOfRangeException extends LogicException {}

class RuntimeException extends Exception {}
class OutOfBoundsException extends RuntimeException {}
class OverflowException extends RuntimeException {}
class RangeException extends RuntimeException {}
class UnderflowException extends RuntimeException {}
class UnexpectedValueException extends RuntimeException {}

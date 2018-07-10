<?hh // decl /* -*- mode: php -*- */
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

interface Throwable {
  public function getMessage(): string;
  // Documented as 'int' in PHP docs, but not actually guaranteed;
  // subclasses (e.g. PDO) can do what they want.
  public function getCode(): mixed;
  public function getFile(): string;
  public function getLine(): int;
  /* HH_FIXME[2082] T30662901 */
  public function getTrace(): array<mixed>;
  public function getTraceAsString(): string;
  public function getPrevious(): ?Throwable;
  public function __toString(): string;
}

class Error implements Throwable {
  protected string $message;
  protected mixed $code;
  protected string $file;
  protected int $line;

  /* Methods */
  <<__Rx>>
  public function __construct (
    string $message = "",
    int $code = 0,
    ?Throwable $previous = null,
  );
  <<__Rx, __MaybeMutable>>
  final public function getMessage(): string;
  final public function getPrevious(): ?Throwable;
  <<__Rx, __MaybeMutable>>
  final public function getCode(): mixed;
  <<__Rx, __MaybeMutable>>
  final public function getFile(): string;
  <<__Rx, __MaybeMutable>>
  final public function getLine(): int;
  /* HH_FIXME[2082] T30662901 */
  final public function getTrace(): array<mixed>;
  final public function getTraceAsString(): string;
  public function __toString(): string;
  final private function __clone(): void;
}

class ArithmeticError extends Error {}
class ArgumentCountError extends Error {}
class AssertionError extends Error {}
class DivisionByZeroError extends Error {}
class ParseError extends Error {}
class TypeError extends Error {}

class Exception implements Throwable {
  protected int $code;
  protected string $file;
  protected int $line;
  /* HH_FIXME[2082] T30662901 */
  protected array $trace;
  protected mixed $userMetadata;

  <<__Rx>>
  public function __construct (
    protected string $message = '',
    int $code = 0,
    protected ?Exception $previous = null,
  );
  <<__Rx, __OnlyRxIfImpl(HH\Rx\Exception::class), __MaybeMutable>>
  public function getMessage(): string;
  final public function getPrevious(): ?Exception;
  public final function setPrevious(Exception $previous): void;
  <<__Rx, __OnlyRxIfImpl(HH\Rx\Exception::class), __MaybeMutable>>
  public function getCode(): int;
  <<__Rx, __MaybeMutable>>
  final public function getFile(): string;
  <<__Rx, __MaybeMutable>>
  final public function getLine(): int;
  /* HH_FIXME[2082] T30662901 */
  final public function getTrace(): array<mixed>;
  /* HH_FIXME[2082] T30662901 */
  final protected function __prependTrace(array $trace): void;
  final public function getTraceAsString(): string;
  public function __toString(): string;
  final private function __clone(): void;

  final public static function getTraceOptions();
  final public static function setTraceOptions($opts);
}

class ErrorException extends Exception {
  <<__Rx>>
  public function __construct(
    $message = "",
    int $code = 0,
    protected int $severity = 0,
    string $filename = '' /* __FILE__ */,
    int $lineno = 0 /* __LINE__ */,
    ?Exception $previous = null
  );
  <<__Rx, __MaybeMutable>>
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

class InvariantException extends Exception {}
final class TypeAssertionException extends Exception {}

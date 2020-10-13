<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

/**
 * This file provides type information for some of HHVM's builtin classes.
 *
 * YOU SHOULD NEVER INCLUDE THIS FILE ANYWHERE!!!
 */
namespace HH {

/**
 * BuiltinEnum contains the utility methods provided by enums.
 * Under the hood, an enum Foo will extend BuiltinEnum<Foo>.
 *
 * HHVM provides a native implementation for this class. The PHP class
 * definition below is not actually used at run time; it is simply
 * provided for the typechecker and for developer reference.
 */
abstract class BuiltinEnum<+T> {
  /**
   * Get the values of the public consts defined on this class,
   * indexed by the string name of those consts.
   *
   * @return array ('CONST_NAME' => $value, ....)
   */
  <<__Pure>>
  final public static function getValues(): darray<string, T>;

  /**
   * Get the names of all the const values, indexed by value. Calls
   * invariant_exception if multiple constants have the same value.
   *
   * @return array($value => 'CONST_NAME', ....)
   */
  <<__Pure>>
  final public static function getNames(): darray<T, string> where T as arraykey;

  /**
   * Returns whether or not the value is defined as a constant.
   */
  <<__Pure>>
  final public static function isValid(mixed $value): bool;

  /**
   * Coerce to a valid value or null.
   * This is useful for typing deserialized enum values.
   */
  <<__Pure>>
  final public static function coerce(mixed $value): ?T;

  /**
   * Coerce to valid value or throw UnexpectedValueException
   * This is useful for typing deserialized enum values.
   */
  <<__Pure>>
  final public static function assert(mixed $value): T;

  /**
   * Coerce all the values in a traversable. If the value is not an
   * array of valid items, an UnexpectedValueException is thrown
   */
  <<__Pure, __AtMostRxAsArgs>>
  final public static function assertAll(
    <<__OnlyRxIfImpl(\HH\Rx\Traversable::class), __MaybeMutable>> Traversable<mixed> $values,
  ): Container<T>;
}

type enumname<T> = classname<BuiltinEnum<T>>;

const enumname<arraykey> BUILTIN_ENUM = BuiltinEnum::class;

/* Experimental section for Enum Classes. This is bound to evolve with
 * the HIP process.
 */

/**
 * Base helper class for the enum class feature
 */
final class Elt<-TPhantom, +T> {
  /* TODO(T77095784) How to make it private ? */
  public function __construct(private string $name, private T $data) {}

  <<__Pure>>
  public function name(): string {
    return $this->name;
  }

  /* TODO(T77095784) ask if this can be inlined/optimized */
  <<__Pure>>
  public function unwrap(): T {
    return $this->data;
  }
}

/**
 * BuiltinEnumClass contains the utility methods provided by enum classes.
 * Under the hood, an enum class Foo : Bar will extend
 * BuiltinEnumClass<HH\Elt<this, Bar>>.
 *
 * HHVM provides a native implementation for this class. The PHP class
 * definition below is not actually used at run time; it is simply
 * provided for the typechecker and for developer reference.
 */
<<__EnumClass>>
abstract class BuiltinEnumClass<+T> {
  /**
   * Get the values of the public consts defined on this class,
   * indexed by the string name of those consts.
   *
   * @return array ('CONST_NAME' => $value, ....)
   */
  <<__Pure>>
  final public static function getValues(): darray<string, T>;
}

}

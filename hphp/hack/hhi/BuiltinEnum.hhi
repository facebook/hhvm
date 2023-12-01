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
     * @return dict['CONST_NAME' => $value, ...]
     */
    <<__NoAutoDynamic>>
    final public static function getValues()[]: darray<string, T>;

    /**
     * Get the names of all the const values, indexed by value. Calls
     * invariant_exception if multiple constants have the same value.
     *
     * @return dict[$value => 'CONST_NAME', ...]
     */
    <<__NoAutoDynamic>>
    final public static function getNames()[]: darray<T, string>
    where
      T as arraykey;

    /**
     * Returns whether or not the value is defined as a constant.
     */
    <<__NoAutoDynamic>>
    final public static function isValid(mixed $value)[]: bool;

    /**
     * Coerce to a valid value or null.
     * This is useful for typing deserialized enum values.
     */
    <<__NoAutoDynamic>>
    final public static function coerce(mixed $value)[]: ?T;

    /**
     * Coerce to valid value or throw UnexpectedValueException
     * This is useful for typing deserialized enum values.
     */
    <<__NoAutoDynamic>>
    final public static function assert(mixed $value)[]: T;

    /**
     * Coerce all the values in a traversable. If the value is not an
     * array of valid items, an UnexpectedValueException is thrown
     */
    <<__NoAutoDynamic, __SupportDynamicType>>
    final public static function assertAll(
      Traversable<mixed> $values,
    )[]: Container<T>;
  }

  type enumname<T> = classname<BuiltinEnum<T>>;

  const enumname<arraykey> BUILTIN_ENUM;

  /**
   * Wrapper for enum class
   */
  newtype MemberOf<-TEnumClass, +TType> as TType = TType;

  /**
   * BuiltinAbstractEnumClass contains the utility methods provided by
   * abstract enum classes.
   *
   * HHVM provides a native implementation for this class. The PHP class
   * definition below is not actually used at run time; it is simply
   * provided for the typechecker and for developer reference.
   */
  <<__EnumClass>>
  abstract class BuiltinAbstractEnumClass {
    final public static function nameOf<TType>(
      \HH\EnumClass\Label<this, TType> $label,
    )[]: string;
  }
  /**
   * BuiltinEnumClass contains the utility methods provided by enum classes.
   * Under the hood, an enum class Foo : Bar will extend
   * BuiltinEnumClass<HH\MemberOf<this, Bar>>.
   *
   * HHVM provides a native implementation for this class. The PHP class
   * definition below is not actually used at run time; it is simply
   * provided for the typechecker and for developer reference.
   */
  <<__EnumClass>>
  abstract class BuiltinEnumClass<+T> extends BuiltinAbstractEnumClass {
    /**
     * Get the values of the public consts defined on this class,
     * indexed by the string name of those consts.
     *
     * @return array ('CONST_NAME' => $value, ....)
     */
    final public static function getValues()[write_props]: darray<string, T>;

    final public static function valueOf<TEnum super this, TType>(
      \HH\EnumClass\Label<TEnum, TType> $label,
    )[write_props]: MemberOf<TEnum, TType>;
  }

}

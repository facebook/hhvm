<?hh // partial

namespace HH {

/**
 * BuiltinEnum contains the utility methods provided by enums.
 * Under the hood, an enum Foo will extend BuiltinEnum<Foo>.
 */
abstract class BuiltinEnum<+T> {
  // We set NoFCallBuiltin on our methods to work around an HHVM bug;
  // when using CallBuiltin, the class pointer isn't properly passed.

  /**
   * Get the values of the public consts defined on this class,
   * indexed by the string name of those consts.
   *
   * @return darray['CONST_NAME' => $value, ....]
   */
  <<__Native>>
  final public static function getValues()[]: darray<string, T>;

  /**
   * Get the names of all the const values, indexed by value. Calls
   * invariant_exception if multiple constants have the same value.
   *
   * @return darray[$value => 'CONST_NAME', ....]
   */
  <<__Native>>
  final public static function getNames()[]: darray<T, string>;

  /**
   * Returns whether or not the value is defined as a constant.
   */
  <<__Native>>
  final public static function isValid(mixed $value)[]: bool;

  /**
   * Coerce to a valid value or null.
   * This is useful for typing deserialized enum values.
   */
  <<__Native>>
  final public static function coerce(mixed $value)[]: ?T;

  /**
   * Coerce to valid value or throw UnexpectedValueException
   * This is useful for typing deserialized enum values.
   */
  final public static function assert(mixed $value)[]: T {
    $new_value = static::coerce($value);
    if (null === $new_value) {
      $cls = static::class;
      throw new \UnexpectedValueException(
        (string)$value." is not a valid value for {$cls}",
      );
    }
    return $new_value;
  }

  /**
   * Coerce all the values in a traversable. If the value is not an
   * array of valid items, an UnexpectedValueException is thrown
   */
  final public static function assertAll(
    Traversable<mixed> $values,
  )[]: Container<T> {
    $new_values = varray[];
    foreach ($values as $value) {
      $new_values[] = static::assert($value);
    }
    return $new_values;
  }
}

type enumname<T> = classname<BuiltinEnum<T>>;
/**
 * Type of atoms
 */
newtype Label<-TEnumClass, +TType> = string;


/**
 * BuiltinEnumClass contains the utility methods provided by enum classes.
 * Under the hood, an enum class Foo : Bar will extend
 * BuiltinEnumClass<HH\MemberOf<this, Bar>>.
 *
 * HHVM provides a native implementation for this class. The PHP class
 * definition below is not actually used at run time; it is simply
 * provided for the typechecker and for developer reference.
 */
abstract class BuiltinEnumClass<+T> {
  /**
   * Get the values of the public consts defined on this class,
   * indexed by the string name of those consts.
   *
   * @return array ('CONST_NAME' => $value, ....)
   */
  <<__Native>>
  final public static function getValues()[write_props]: darray<string, T>;

  final public static function nameOf(Label<this, mixed> $atom): string {
    return $atom;
  }

  final public static function valueOf<TEnum super this, TType>(Label<TEnum, TType> $atom): MemberOf<TEnum, TType> {
    return \__SystemLib\get_enum_member_by_label($atom);
  }
}


}

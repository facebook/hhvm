<?hh

namespace HH {

/**
 * BuiltinEnum contains the utility methods provided by enums.
 * Under the hood, an enum Foo will extend BuiltinEnum<Foo>.
 */
abstract class BuiltinEnum<T> {
  // We set NoFCallBuiltin on our methods to work around an HHVM bug;
  // when using CallBuiltin, the class pointer isn't properly passed.

  /**
   * Get the values of the public consts defined on this class,
   * indexed by the string name of those consts.
   *
   * @return array ('CONST_NAME' => $value, ....)
   */
  <<__Native>>
  final public static function getValues(): array<string, T>;

  /**
   * Get the names of all the const values, indexed by value. Calls
   * invariant_exception if multiple constants have the same value.
   *
   * @return array($value => 'CONST_NAME', ....)
   */
  <<__Native>>
  final public static function getNames(): array<T, string>;

  /**
   * Returns whether or not the value is defined as a constant.
   */
  <<__Native>>
  final public static function isValid(mixed $value): bool;

  /**
   * Coerce to a valid value or null.
   * This is useful for typing deserialized enum values.
   */
  <<__Native>>
  final public static function coerce(mixed $value): ?T;

  /**
   * Coerce to valid value or throw UnexpectedValueException
   * This is useful for typing deserialized enum values.
   */
  final public static function assert(mixed $value): T {
    $new_value = static::coerce($value);
    if (null === $new_value) {
      $cls = \get_called_class();
      throw new \UnexpectedValueException(
        "{$value} is not a valid value for {$cls}",
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
  ): Container<T> {
    $new_values = array();
    foreach ($values as $value) {
      $new_values[] = static::assert($value);
    }
    return $new_values;
  }
}

}

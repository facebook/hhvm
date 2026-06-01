<?hh
// Test that self-referential abstract type constants (T as this::T) work
// correctly with SDT-requiring generics like Awaitable and GenericEnumClass.

abstract class SelfRefAwaitable {
  abstract const type T as this::T;
  public static function get(): this::T {
    throw new Exception();
  }
  public static async function gen(): Awaitable<this::T> {
    throw new Exception();
  }
}

abstract class SelfRefEnumClass {
  abstract const type TEnum as HH\GenericEnumClass<this::TEnum, string>;
}

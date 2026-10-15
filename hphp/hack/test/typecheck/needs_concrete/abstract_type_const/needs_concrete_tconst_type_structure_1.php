<?hh

abstract class Base {
  abstract const type T;

  public static function f(): void {
    // error: abstract type constant resolved through `static`, which might
    // refer to an abstract class at runtime
    $_ = type_structure(static::class, 'T');
  }

  <<__NeedsConcrete>>
  public static function g(): void {
    // ok: `static` points to a concrete class in a `__NeedsConcrete` method
    $_ = type_structure(static::class, 'T');
  }

  public function h(): void {
    // ok: `static` points to a concrete class in an instance method
    $_ = type_structure(static::class, 'T');
  }
}

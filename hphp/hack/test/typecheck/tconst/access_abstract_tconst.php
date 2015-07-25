<?hh // strict

// Accessing an abstract type constant is not allowed

abstract class C {
  abstract const type T;

  public static function foo(): this::T {
    throw new Exception('');
  }
}

function test(): void {
  C::foo();
}

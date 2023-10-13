<?hh

interface I {
  public static function f(): void;
}

function test<T as I>(classname<T> $c): void {
  $c::f();
}

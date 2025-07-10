<?hh

interface I {
  public static function f(): void;
}

function test<T as I>(class<T> $c): void {
  $c::f();
}

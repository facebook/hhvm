<?hh // strict

interface I {
  public static function f<T>(): ?T;
}

function test(classname<I> $classname): nonnull {
  return $classname::f();
}

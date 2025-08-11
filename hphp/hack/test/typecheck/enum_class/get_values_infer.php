<?hh

enum class EC: I {}

interface I {
  public static function foo(): void;
}

function test(): void {
  $vs = EC::getValues();
  each($vs, $x ==> $x::foo());
}

function each<T>(Traversable<T> $xs, (function(T): void) $f): void {}

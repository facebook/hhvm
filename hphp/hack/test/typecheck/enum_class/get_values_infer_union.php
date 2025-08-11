<?hh

enum class EC: I {}

enum class ED: I {}

interface I {
  public static function foo(): void;
  public function bar(): void;
}

function test(bool $b): void {
  $vs = $b ? EC::getValues() : ED::getValues();
  each($vs, $x ==> $x->bar());
}

function each<T>(Traversable<T> $xs, (function(T): void) $f): void {}

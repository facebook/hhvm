<?hh // partial

interface X<+T> {
  public function test((function(T): void) $x): void;
}

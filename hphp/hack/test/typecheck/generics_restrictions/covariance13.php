<?hh // partial

interface X<+T> {
  public function test(T $x): void;
  public function another((function():T) $f): void;
}

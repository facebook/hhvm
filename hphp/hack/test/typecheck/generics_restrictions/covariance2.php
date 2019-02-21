<?hh // partial

interface X<+T> {
  public function test(T $x): void;
}

<?hh // partial

interface X<+T> {
  public function test(T $x): void;
  public function test2(T... $x): void;
}

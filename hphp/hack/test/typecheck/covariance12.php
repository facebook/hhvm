<?hh

interface X<+T as int> {
  public function test(T $x): void;
}

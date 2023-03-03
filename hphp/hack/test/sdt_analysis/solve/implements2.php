<?hh

interface I {
  public function foo(vec<int> $_): void;
}

class C implements I {
  public function foo(vec<int> $_): void {}
}

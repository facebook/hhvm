<?hh

interface I {
  public function foo(vec<int> $_): void;
}

final class C implements I {
  public function foo(vec<int> $_): void {}
}

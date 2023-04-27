<?hh
// this test shows that the same class-like item
// (in this case I) can be included in multiple
// `__NoAutoDynamic`able lists
interface I {
  public function foo(vec<int> $_): void;
}

final class C implements I {
  public function foo(vec<int> $_): void {}
}

final class D implements I {
  public function foo(vec<int> $_): void {}
}

final class E {}

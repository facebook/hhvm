<?hh
class C {
  public function blah(bool $x): ?D {
    return $x ? new D() : null;
  }
}
class D {
  public function bar(): E {
    return new E();
  }
}
class E {}
function test(?C $c, bool $b): ?E {
  return $c?->blah($b)->bar();
}

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
class E {
  public function baz(): F {
    return new F();
  }
}
class F {}
function test(C $c, bool $b): ?F {
  return $c->blah($b)?->bar()->baz();
}

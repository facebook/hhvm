<?hh // strict
class C {
  public ?D $prop;
  public function foo(): D {
    return new D();
  }
  public function blah(bool $x): ?D {
    return null;
  }
  public function yar(int $x): void {}
}
class D {
  public function bar(): E {
    return new E();
  }
  public function nar(): ?E {
    return null;
  }
}
class E {
  public function baz(): F {
    return new F();
  }
}
class F {}
function test1(?C $c): ?D {
  return $c?->foo();
}
function test2(?C $c): ?F {
  return $c?->foo()?->bar()?->baz();
}
function test3(C $c, bool $b): ?E {
  return $c->blah($b)?->bar();
}
function test4(C $c): void {
  if ($c->prop !== null) {
    $c->prop->nar()?->baz();
  }
}

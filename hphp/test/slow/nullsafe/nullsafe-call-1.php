<?hh
class C {
  public ?D $prop;
  public function foo(): D {
    return new D();
  }
  public function blah(bool $x): ?D {
    return $x ? new D() : null;
  }
  public function yar(int $x): void {}
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
  var_dump($c->prop?->bar()?->baz());
  if ($c->prop !== null) {
    var_dump($c->prop->bar()?->baz());
  } else {
    var_dump(null);
  }
}
function test5(?C $c): int {
  $x = null;
  $c?->yar($x = 123);
  return $x;
}
function main() :mixed{
  $c = new C();
  echo "test1:\n";
  var_dump(test1($c));
  var_dump(test1(null));
  echo "test2:\n";
  var_dump(test2($c));
  var_dump(test2(null));
  echo "test3:\n";
  var_dump(test3($c, true));
  var_dump(test3($c, false));
  echo "test4:\n";
  $c->prop = new D();
  var_dump(test4($c));
  $c->prop = null;
  var_dump(test4($c));
  echo "test5:\n";
  var_dump(test5($c));
  var_dump(test5(null));
  echo "Done\n";
}

<<__EntryPoint>>
function main_nullsafe_call_1() :mixed{
main();
}

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
function main() :mixed{
  $c = new C();
  var_dump(test($c, true));
  try {
    var_dump(test($c, false));
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
}

<<__EntryPoint>>
function main_nullsafe_call_4() :mixed{
main();
}

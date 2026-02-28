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
function test1(?C $c, bool $b): ?E {
  return $c?->blah($b)->bar();
}
function test2(?C $c, bool $b): ?F {
  return $c?->blah($b)->bar()?->baz();
}
function main() :mixed{
  $c = new C();
  var_dump(test1($c, true));
  try {
    var_dump(test1(null, true));
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
  try {
    var_dump(test1($c, false));
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
  try {
    var_dump(test1(null, false));
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
  var_dump(test2($c, true));
  try {
    var_dump(test2(null, true));
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
  try {
    var_dump(test2($c, false));
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
  try {
    var_dump(test2(null, false));
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
}

<<__EntryPoint>>
function main_nullsafe_call_3() :mixed{
main();
}

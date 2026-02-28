<?hh
class D {
  public function bar() :mixed{
    echo "D::bar() was called\n";
  }
}
class E {
  public function bar() :mixed{
    echo "E::bar() was called\n";
  }
}
class C {
  public function foo() :mixed{
    return new D();
  }
}
function f($b) :mixed{
  return $b ? new C() : null;
}
function test($b) :mixed{
  $x = f($b)?->foo();
  if ($x !== null) {
    return $x;
  }
  return new E();
}
function main() :mixed{
  echo "1:\n";
  $x = test(false);
  $x->bar();
  echo "2:\n";
  $x = test(true);
  $x->bar();
  echo "Done\n";
}

<<__EntryPoint>>
function main_nullsafe_call_5() :mixed{
main();
}

<?hh
class E {
  public function blah() :mixed{
    echo "E::blah() was called\n";
  }
}
class D {
  public function bar() :mixed{
    return new E();
  }
}
class C {
  public function foo() :mixed{
    return new D();
  }
}
function test($obj) :mixed{
  $x = $obj?->foo()?->bar();
  if ($x !== null) {
    $x->blah();
  }
}
function main() :mixed{
  echo "1:\n";
  test(null);
  echo "2:\n";
  test(new C());
  echo "Done\n";
}

<<__EntryPoint>>
function main_nullsafe_call_9() :mixed{
main();
}

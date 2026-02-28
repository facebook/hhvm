<?hh
class D {
  public function bar() :mixed{
    echo "D::bar() was called\n";
  }
}
class C {
  public function foo() :mixed{
    return new D();
  }
}
function test($obj) :mixed{
  $d = $obj?->foo();
  if ($d !== null) {
    $d->bar();
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
function main_nullsafe_call_8() :mixed{
main();
}

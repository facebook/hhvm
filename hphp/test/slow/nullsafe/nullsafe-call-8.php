<?hh
class D {
  public function bar() {
    echo "D::bar() was called\n";
  }
}
class C {
  public function foo() {
    return new D();
  }
}
function test($obj) {
  $d = $obj?->foo();
  if ($d !== null) {
    $d->bar();
  }
}
function main() {
  echo "1:\n";
  test(null);
  echo "2:\n";
  test(new C());
  echo "Done\n";
}

<<__EntryPoint>>
function main_nullsafe_call_8() {
main();
}

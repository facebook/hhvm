<?hh
class D {
  public function bar() {
    echo "D::bar() was called\n";
  }
}
class E {
  public function bar() {
    echo "E::bar() was called\n";
  }
}
class C {
  public function foo() {
    return new D();
  }
}
function f($b) {
  return $b ? new C() : null;
}
function test($b) {
  $x = f($b)?->foo();
  if ($x !== null) {
    return $x;
  }
  return new E();
}
function main() {
  echo "1:\n";
  $x = test(false);
  $x->bar();
  echo "2:\n";
  $x = test(true);
  $x->bar();
  echo "Done\n";
}

<<__EntryPoint>>
function main_nullsafe_call_5() {
main();
}

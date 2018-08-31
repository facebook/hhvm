<?hh
class E {
  public function blah() {
    echo "E::blah() was called\n";
  }
}
class D {
  public function bar() {
    return new E();
  }
}
class C {
  public function foo() {
    return new D();
  }
}
function test($obj) {
  $x = $obj?->foo()?->bar();
  if ($x !== null) {
    $x->blah();
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
function main_nullsafe_call_9() {
main();
}

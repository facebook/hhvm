<?hh
class C {
  public function foo() {
    echo "foo() was called\n";
    return $this;
  }
  public function bar() {
    echo "bar() was called\n";
    return $this;
  }
}
function f() {
  echo "f() was called\n";
}
function g() {
  echo "g() was called\n";
}
function main() {
  $obj = new C();
  $x = $obj?->foo(f())?->bar(g());
  var_dump($x);
  $obj = null;
  $x = $obj?->foo(f())?->bar(g());
  var_dump($x);
  echo "Done\n";
}

<<__EntryPoint>>
function main_nullsafe_call_7() {
main();
}

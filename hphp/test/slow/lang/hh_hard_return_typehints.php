<?hh
error_reporting(-1);
function handler($errno, $errmsg) {
  if ($errno === E_RECOVERABLE_ERROR) {
    echo "Triggered E_RECOVERABLE_ERROR: $errmsg\n";
  } else if ($errno === E_WARNING) {
    echo "Triggered E_WARNING: $errmsg\n";
  } else {
    echo "$errno: $errmsg\n";
  }
  return true;
}
set_error_handler(fun('handler'));
class Foo {
  public function go() { echo "Foo::go()\n"; }
}
class Bar {
  public function go() { echo "Bar::go()\n"; }
}
function test1(): @Foo {
  return new Foo();
}
function test2(): Foo {
  return new Foo();
}
function test3(): @Bar {
  return new Foo();
}
function test4(): Bar {
  return new Foo();
}
function main(): void {
  test1()->go();
  test2()->go();
  test3()->go();
  test4()->go();
}
main();

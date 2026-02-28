<?hh
function handler($errno, $errmsg) :mixed{
  if ($errno === E_RECOVERABLE_ERROR) {
    echo "Triggered E_RECOVERABLE_ERROR: $errmsg\n";
  } else if ($errno === E_WARNING) {
    echo "Triggered E_WARNING: $errmsg\n";
  } else {
    echo "$errno: $errmsg\n";
  }
  return true;
}
class Foo {
  public function go() :mixed{ echo "Foo::go()\n"; }
}
class Bar {
  public function go() :mixed{ echo "Bar::go()\n"; }
}
function test1(): <<__Soft>> Foo {
  return new Foo();
}
function test2(): Foo {
  return new Foo();
}
function test3(): <<__Soft>> Bar {
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
<<__EntryPoint>>
function entrypoint_hh_hard_return_typehints(): void {
  error_reporting(-1);
  set_error_handler(handler<>);
  main();
}

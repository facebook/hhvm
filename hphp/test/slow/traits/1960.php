<?hh

trait T {
  function foo() {
    echo "Foo";
    $this->bar();
  }
  abstract function bar();
}
class C {
  use T;
  function bar() {
    echo "BAR!\n";
  }
}
<<__EntryPoint>> function main(): void {
$x = new C();
$x->foo();
}

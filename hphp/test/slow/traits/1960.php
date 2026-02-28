<?hh

trait T {
  function foo() :mixed{
    echo "Foo";
    $this->bar();
  }
  abstract function bar():mixed;
}
class C {
  use T;
  function bar() :mixed{
    echo "BAR!\n";
  }
}
<<__EntryPoint>> function main(): void {
$x = new C();
$x->foo();
}

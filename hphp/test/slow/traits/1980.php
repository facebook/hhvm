<?hh

trait T {
  function foo() {
    echo "Foo";
    parent::bar();
    echo "I'm in class " . get_class() . "\n";
  }
}
class C {
  function bar() {
    echo "BAR!\n";
  }
}
class D extends C {
  use T;
}
<<__EntryPoint>> function main(): void {
$x = new D();
$x->foo();
}

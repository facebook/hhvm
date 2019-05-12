<?hh

abstract class A {
  protected abstract function foo();
}

class B extends A {
  protected function foo() {
    echo "B::foo\n";
  }
}

class C extends A {
  function foo() {
    $b = new B;
    $b->foo();
  }
}
<<__EntryPoint>> function main(): void {
$c = new C;
$c->foo();
}

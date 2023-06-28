<?hh

abstract class A {
  protected abstract function foo():mixed;
}

class B extends A {
  protected function foo() :mixed{
    echo "B::foo\n";
  }
}

class C extends A {
  function foo() :mixed{
    $b = new B;
    $b->foo();
  }
}
<<__EntryPoint>> function main(): void {
$c = new C;
$c->foo();
}

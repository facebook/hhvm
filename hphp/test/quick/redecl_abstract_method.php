<?hh

abstract class A {
  abstract function f():mixed;
}

abstract class B extends A {
  abstract function f():mixed;
}

class C extends B {
  public function f():mixed{
    echo "Foo\n";
  }
}

<<__EntryPoint>> function main(): void {
  $c = new C;
  $c->f();
}

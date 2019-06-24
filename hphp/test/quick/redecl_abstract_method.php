<?hh

abstract class A {
  abstract function f();
}

abstract class B extends A {
  abstract function f();
}

class C extends B {
  public function f(){
    echo "Foo\n";
  }
}

<<__EntryPoint>> function main(): void {
  $c = new C;
  $c->f();
}

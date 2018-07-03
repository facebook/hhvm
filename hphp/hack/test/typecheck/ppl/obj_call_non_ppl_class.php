<?hh // strict

<<__PPL>>
class MyClass {
  public function myMethod(): void {}
}

class NonPPLClass {
  public function error(): void {
    $x = new MyClass();
    $x->myMethod();
  }
}

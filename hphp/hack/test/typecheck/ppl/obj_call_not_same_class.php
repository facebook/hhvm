<?hh // strict

<<__PPL>>
class MyClass {
  public function myMethod(): void {}
}

<<__PPL>>
class OtherPPLClass {
  public function error(): void {
    $x = new MyClass();
    $x->myMethod();
  }
}

<?hh // strict

<<__PPL>>
class MyClass {
  public function myMethod(): void {}

  public function correct(): void {
    $this->myMethod();
  }
}

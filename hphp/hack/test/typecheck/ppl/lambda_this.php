<?hh // strict

<<__PPL>>
class MyClass {
  public function foo(): void {}

  public function my_lambda(): void {
    $x = () ==> {
      $this->foo();
    };
  }
}

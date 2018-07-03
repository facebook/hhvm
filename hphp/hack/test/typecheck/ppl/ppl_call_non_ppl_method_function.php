<?hh // strict

class MyRegularClass {
  public function totallyOkay(): void {}
}

function regular_function(): void {}

<<__PPL>>
class MyClass {
  public function callOther(): void {
    $foo = new MyRegularClass();
    $foo->totallyOkay();

    regular_function();

    return;
  }
}

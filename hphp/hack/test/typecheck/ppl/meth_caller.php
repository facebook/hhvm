<?hh // strict

<<__PPL>>
class MyClass {
  public function foo(): void {}

  public function meth_caller_user(): void {
    $a = meth_caller('MyClass', 'foo');
    $a($this);
  }
}

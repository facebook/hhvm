<?hh // strict

<<__PPL>>
class MyClass {
  public function foo(): void {}

  public function inst_meth_user(): void {
    $a = inst_meth($this, 'foo');
    $a();
  }
}

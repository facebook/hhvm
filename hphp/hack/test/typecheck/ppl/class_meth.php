<?hh // strict

<<__PPL>>
class MyClass {
  public static function foo(): void {}

  public function class_meth(): void {
    $a = class_meth('MyClass', 'foo');
    $a();
  }
}

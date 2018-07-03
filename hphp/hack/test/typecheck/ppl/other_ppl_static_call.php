<?hh // strict

<<__PPL>>
class MyClassWithStaticMethod {
  public static function shouldNotWork(): void {}
}

<<__PPL>>
class MyClass {
  public function correct(): void {
    $x = new MyClassWithStaticMethod();
    $x::shouldNotWork();
  }
}

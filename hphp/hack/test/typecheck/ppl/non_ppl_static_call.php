<?hh // strict

class MyClassWithStaticMethod {
  public static function shouldWork(): void {}
}

<<__PPL>>
class MyClass {
  public function correct(): void {
    $x = new MyClassWithStaticMethod();
    $x::shouldWork();
  }
}

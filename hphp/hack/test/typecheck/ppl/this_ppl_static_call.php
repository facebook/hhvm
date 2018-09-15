<?hh // strict

<<__PPL>>
class MyClass {
  public static function staticMethod(): void {}

  public function correct(): void {
    $this::staticMethod();
  }
}

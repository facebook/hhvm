<?hh // strict

<<__PPL>>
class MyClass {
  public static function staticMeth(): void {
  }
}

<<__PPL>>
class PPLClass {
  public function staticCall(): void {
    MyClass::staticMeth();
  }
}

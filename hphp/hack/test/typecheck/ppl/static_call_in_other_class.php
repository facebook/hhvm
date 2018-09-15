<?hh // strict

<<__PPL>>
class MyClass {
  public static function staticMeth(): void {
  }
}

class NonPPLClass {
  public function staticCall(): void {
    MyClass::staticMeth();
  }
}

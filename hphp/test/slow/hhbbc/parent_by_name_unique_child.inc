<?hh

class D extends C {
  // incompatible with the second version of C
  const type INCOMPATIBLE = int;

  public static function printStuff() {
    echo C::C['c'];
  }
}

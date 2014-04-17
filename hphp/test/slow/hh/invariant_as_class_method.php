<?hh

class A {
  public static function invariant() {
    var_dump("works");
  }
}
A::invariant();

<?hh

class A {
  function a1() {} //error2054
  static function a2() {}
  protected static function a3() {}
}

interface B {
  function b1();  //error2054
  final function b2();
  public final function b3();
}

trait C {
  function c1() {}  //error2054
  static function c2() {}
  private static function c3() {}
}

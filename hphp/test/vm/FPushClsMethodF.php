<?php
class B {
  public static function g1() {
    static::h();
  }
  public static function h() {
    echo "B\n";
  }
}
class C extends B {
  public static function f() {
    B::g1();
    parent::g1();
    C::g2();
    self::g2();
  }
  public static function g2() {
    static::h();
  }
  public static function h() {
    echo "C\n";
  }
}
class D extends C {
  public static function h() {
    echo "D\n";
  }
}
D::f();


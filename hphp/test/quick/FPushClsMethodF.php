<?hh
class B {
  public static function g1() :mixed{
    static::h();
  }
  public static function h() :mixed{
    echo "B\n";
  }
}
class C extends B {
  public static function f() :mixed{
    B::g1();
    parent::g1();
    C::g2();
    self::g2();
  }
  public static function g2() :mixed{
    static::h();
  }
  public static function h() :mixed{
    echo "C\n";
  }
}
class D extends C {
  public static function h() :mixed{
    echo "D\n";
  }
}
<<__EntryPoint>> function main(): void {
D::f();
}

<?hh

trait T {
  public static function a() {}
  public function b() {}
}

class C {
  use T;
  public static function a() = T::a;
  public static function b() = T::b;
}


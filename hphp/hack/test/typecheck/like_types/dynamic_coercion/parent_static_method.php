<?hh // partial

class C {
  public static function f(int $i): void {}
}
// Calls to parent methods are handled separately in a static and instance context
class D extends C {
  public function g(dynamic $d): void {
    parent::f($d);
  }
  public static function g_static(dynamic $d): void {
    parent::f($d);
  }
}

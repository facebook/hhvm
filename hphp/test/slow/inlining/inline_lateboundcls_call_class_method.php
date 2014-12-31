<?

abstract class A {
  static function heh() {
    return static::go();
  }
  abstract static function go();
}
class B extends A {
  static function go() {
    echo "hi\n";
  }
}
function x() {
  B::heh();
}

x();

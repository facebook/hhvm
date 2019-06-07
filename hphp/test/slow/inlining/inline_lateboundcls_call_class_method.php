<?hh

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


<<__EntryPoint>>
function main_inline_lateboundcls_call_class_method() {
x();
}

<?hh

abstract class A {
  static function heh() :mixed{
    return static::go();
  }
  abstract static function go():mixed;
}
class B extends A {
  static function go() :mixed{
    echo "hi\n";
  }
}
function x() :mixed{
  B::heh();
}


<<__EntryPoint>>
function main_inline_lateboundcls_call_class_method() :mixed{
x();
}

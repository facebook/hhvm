<?hh

class X { static function y() {} }
function t() { var_dump(__FUNCTION__); }

<<__EntryPoint>>
function main() {
  (new ReflectionFunction('t'))->invoke();
  var_dump(__hhvm_intrinsics\create_class_pointer(__hhvm_intrinsics\create_class_pointer('X')));
}

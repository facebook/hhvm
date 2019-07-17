<?hh

class X { static function y() {} }
function t() { var_dump(__FUNCTION__); }

<<__EntryPoint>>
function main() {
  (new ReflectionFunction(fun('t')))->invoke();
  var_dump(__hhvm_intrinsics\create_class_pointer(__hhvm_intrinsics\create_class_pointer('X')));
  var_dump(__hhvm_intrinsics\dummy_varray_builtin(class_meth('X', 'y')));
}

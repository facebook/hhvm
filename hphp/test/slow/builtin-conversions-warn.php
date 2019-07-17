<?hh

class X { static function y() {} }
function t() { var_dump(__FUNCTION__); }

<<__EntryPoint>>
function main() {
  set_error_handler(($_n, $str) ==> { echo "Warning: $str\n"; return true; });

  (new ReflectionFunction(fun('t')))->invoke();
  var_dump(__hhvm_intrinsics\create_class_pointer(__hhvm_intrinsics\create_class_pointer('X')));
  var_dump(__hhvm_intrinsics\dummy_varray_builtin(class_meth('X', 'y')));
}

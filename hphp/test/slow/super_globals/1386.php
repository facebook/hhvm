<?hh

class X {
  static function test() :mixed{
    var_dump(__FUNCTION__);
    var_dump(__CLASS__);
    var_dump(__METHOD__);
    return vec[\HH\global_get(__FUNCTION__),                 \HH\global_get(__CLASS__),                 \HH\global_get(__METHOD__)];
  }
}
<<__EntryPoint>>
function entrypoint_1386(): void {
  \HH\global_set('test', 'this_is_function_test');
  \HH\global_set('X', 'this_is_class_x');
  \HH\global_set('X::test', 'this_is_method_test::x');
  var_dump(X::test());
}

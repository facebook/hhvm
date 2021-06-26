<?hh
class A {
  public static function f($value) {
    $filter = 'g';
    return call_user_func(varray[\HH\global_get('a'), $filter], $value);
  }
  <<__DynamicallyCallable>> public static function g($value) {
    return $value;
  }
}
<<__EntryPoint>>
function entrypoint_743(): void {
  \HH\global_set('a', 'A');
  var_dump(A::f('test'));
}

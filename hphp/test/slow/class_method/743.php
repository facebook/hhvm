<?hh
class A {
  public static function f($value) :mixed{
    $filter = 'g';
    return call_user_func(vec[\HH\global_get('a'), $filter], $value);
  }
  <<__DynamicallyCallable>> public static function g($value) :mixed{
    return $value;
  }
}
<<__EntryPoint>>
function entrypoint_743(): void {
  \HH\global_set('a', 'A');
  var_dump(A::f('test'));
}

<?hh
class A {
  public static function f($value) {
    $filter = 'g';
    return call_user_func(varray[$GLOBALS['a'], $filter], $value);
  }
  public static function g($value) {
    return $value;
  }
}
<<__EntryPoint>>
function entrypoint_743(): void {
  $GLOBALS['a'] = 'A';
  var_dump(A::f('test'));
}

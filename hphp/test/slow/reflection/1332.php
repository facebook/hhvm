<?hh

class A {
  <<__DynamicallyCallable>> public static function f($a) {
    return 'ok'.$a;
  }
}

<<__EntryPoint>>
function main_1332() {
  $obj = new A();
  var_dump(method_exists($obj, 'f'));
  var_dump(method_exists('A', 'f'));
  var_dump(is_callable(varray[$obj, 'f']));
  var_dump(is_callable(varray['A', 'f']));
  var_dump(call_user_func(varray[$obj,'f'], 'blah'));
  var_dump(call_user_func_array(varray[$obj,'f'], varray['blah']));
  var_dump(call_user_func(varray['A','f'], 'blah'));
  var_dump(call_user_func_array(varray['A','f'], varray['blah']));
}

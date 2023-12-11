<?hh

class A {
  <<__DynamicallyCallable>> public static function f($a) :mixed{
    return 'ok'.$a;
  }
}

<<__EntryPoint>>
function main_1332() :mixed{
  $obj = new A();
  var_dump(method_exists($obj, 'f'));
  var_dump(method_exists('A', 'f'));
  var_dump(is_callable(vec[$obj, 'f']));
  var_dump(is_callable(vec['A', 'f']));
  var_dump(call_user_func(vec[$obj,'f'], 'blah'));
  var_dump(call_user_func_array(vec[$obj,'f'], vec['blah']));
  var_dump(call_user_func(vec['A','f'], 'blah'));
  var_dump(call_user_func_array(vec['A','f'], vec['blah']));
}

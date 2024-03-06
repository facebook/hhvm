<?hh
<<__DynamicallyCallable>>
function test($a): mixed {
  return 'ok'.$a;
}


<<__EntryPoint>>
function main_1330() :mixed{
  var_dump(function_exists('test'));
  var_dump(is_callable('test'));
  var_dump(call_user_func(test<>, 'blah'));
  var_dump(call_user_func_array(test<>, vec['blah']));
}

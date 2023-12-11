<?hh
<<__DynamicallyCallable>>
function test($a) :mixed{
 return 'ok'.$a;
}


<<__EntryPoint>>
function main_1330() :mixed{
var_dump(function_exists('TEst'));
 var_dump(is_callable('teSt'));
var_dump(call_user_func(teST<>, 'blah'));
 var_dump(call_user_func_array(teST<>, vec['blah']));
}

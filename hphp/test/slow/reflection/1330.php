<?hh

function test($a) {
 return 'ok'.$a;
}


<<__EntryPoint>>
function main_1330() {
var_dump(function_exists('TEst'));
 var_dump(is_callable('teSt'));
var_dump(call_user_func(fun('teST'), 'blah'));
 var_dump(call_user_func_array(fun('teST'), varray['blah']));
}

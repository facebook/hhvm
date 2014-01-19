<?php

function test($a) {
 return 'ok'.$a;
}
var_dump(function_exists('TEst'));
 var_dump(is_callable('teSt'));
var_dump(call_user_func('teST', 'blah'));
 var_dump(call_user_func_array('teST', array('blah')));


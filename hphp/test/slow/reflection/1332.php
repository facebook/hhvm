<?php

class A {
 public static function f($a) {
 return 'ok'.$a;
}
}
 $obj = new A();
 var_dump(method_exists($obj, 'f'));
var_dump(method_exists('A', 'f'));
var_dump(is_callable(array($obj, 'f')));
var_dump(is_callable(array('A', 'f')));
var_dump(call_user_func(array($obj,'f'), 'blah'));
var_dump(call_user_func_array(array($obj,'f'), array('blah')));
var_dump(call_user_func(array('A','f'), 'blah'));
var_dump(call_user_func_array(array('A','f'), array('blah')));

<?php

function __autoload($class) {
	var_dump($class);
}

call_user_func(array('foo', 'bar'));
call_user_func(array('', 'bar'));
call_user_func(array($foo, 'bar'));
call_user_func(array($foo, ''));

?>
<?php

$foo = function() {
	static $instance;
	
	if (is_null($instance)) {
		$instance = function () {
			return 'OK!';
		};
	}
		
	return $instance;	
};

var_dump(call_user_func(array($foo, '__invoke'))->__invoke());
var_dump(call_user_func(function() use (&$foo) { return $foo; }, '__invoke'));

?>
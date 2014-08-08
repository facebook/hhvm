<?php

spl_autoload_register(function($classname) {
	if (in_array($classname, array('a','b','c'))) {
		require_once ($classname . '.php');
	}
});

set_error_handler(function ($errno, $errstr, $errfile, $errline) {
}, error_reporting());

a::staticTest();

$b = new b();
$b->test();


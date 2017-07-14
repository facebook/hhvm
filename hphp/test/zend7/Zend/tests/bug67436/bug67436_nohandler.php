<?php

spl_autoload_register(function($classname) {
	if (in_array($classname, array('a','b','c'))) {
		require_once __DIR__ . "/{$classname}.php";
	}
});

a::staticTest();

$b = new b();
$b->test();


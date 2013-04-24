<?php

function __autoload($class_name)
{
	require_once(dirname(__FILE__) . '/' . strtolower($class_name) . '.p5c');
	echo __FUNCTION__ . '(' . $class_name . ")\n";
}

var_dump(interface_exists('autoload_interface', false));
var_dump(class_exists('autoload_implements', false));

$o = unserialize('O:19:"Autoload_Implements":0:{}');

var_dump($o);
var_dump($o instanceof autoload_interface);
unset($o);

var_dump(interface_exists('autoload_interface', false));
var_dump(class_exists('autoload_implements', false));

?>
===DONE===
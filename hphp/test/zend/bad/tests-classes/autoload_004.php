<?php

function __autoload($class_name)
{
	var_dump(class_exists($class_name));
	require_once(dirname(__FILE__) . '/' . $class_name . '.p5c');
	echo __FUNCTION__ . '(' . $class_name . ")\n";
}

var_dump(class_exists('autoload_derived'));

?>
===DONE===
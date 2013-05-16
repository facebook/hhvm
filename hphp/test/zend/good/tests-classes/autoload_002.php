<?php

function __autoload($class_name)
{
	require_once(dirname(__FILE__) . '/' . $class_name . '.p5c');
	echo __FUNCTION__ . '(' . $class_name . ")\n";
}

var_dump(get_class_methods('autoload_root'));

?>
===DONE===
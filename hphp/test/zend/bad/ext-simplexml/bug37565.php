<?php

function my_error_handler($errno, $errstr, $errfile, $errline) {
	    echo "Error: $errstr\n";
}

set_error_handler('my_error_handler');

class Setting extends ReflectionObject
{
}

Reflection::export(simplexml_load_string('<test/>', 'Setting'));

Reflection::export(simplexml_load_file('data:,<test/>', 'Setting'));

?>
===DONE===
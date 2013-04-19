<?php

function __autoload($class_name)
{
	var_dump(class_exists($class_name, false));
	require_once(dirname(__FILE__) . '/' . $class_name . '.p5c');
	echo __FUNCTION__ . '(' . $class_name . ")\n";
}

var_dump(class_exists('autoload_derived', false));
var_dump(class_exists('autoload_derived', false));

class Test
{
    function __destruct() {
        echo __METHOD__ . "\n";
        $o = new autoload_derived;
        var_dump($o);
    }
}

$o = new Test;
unset($o);

?>
===DONE===
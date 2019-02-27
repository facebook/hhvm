<?php
$foo = array(1);

function foobar($errno, $errstr, $errfile, $errline) { }

set_error_handler('foobar');

function test($x) {
	global $foo;

	try { $x->invokeArgs(array(0)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

$x = new ReflectionFunction('str_pad');
test($x);
?>
DONE

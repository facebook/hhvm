<?php
ZendGoodZendTestsBug46106::$foo = array(1);

function foobar($errno, $errstr, $errfile, $errline) { }

set_error_handler('foobar');

function test($x) {


	try { $x->invokeArgs(array(0)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

$x = new ReflectionFunction('str_pad');
test($x);

abstract final class ZendGoodZendTestsBug46106 {
  public static $foo;
}
echo "DONE";

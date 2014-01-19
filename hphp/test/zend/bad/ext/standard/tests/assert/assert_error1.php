<?php
function f1() 
{
	echo "f1 called\n";
}
function handler($errno, $errstr) {
        echo "in handler()\n\n";
        assert(E_RECOVERABLE_ERROR === $errno);
        var_dump($errstr);
}

//Wrong number of parameters for assert_options()
assert_options(ASSERT_WARNING, 1);
var_dump($rao = assert_options(ASSERT_CALLBACK, "f1", 1));


//Unknown option for assert_options()
var_dump($rao=assert_options("F1", "f1"));

//Wrong number of parameters for  assert()
$sa="0 != 0";
var_dump($r2 = assert($sa, "message", 1));


//Catch recoverable error with handler
var_dump($rc = assert('aa=sd+as+safsafasfaçsafçsafç'));
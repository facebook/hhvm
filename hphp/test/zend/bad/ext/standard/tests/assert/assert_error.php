<?php
function f1() 
{
	echo "f1 called\n";
}

$sa = "threemeninaboat";

var_dump($r2=assert($sa));

var_dump($ra0 = assert_options(ASSERT_QUIET_EVAL, 1));

var_dump($r2=assert($sa));
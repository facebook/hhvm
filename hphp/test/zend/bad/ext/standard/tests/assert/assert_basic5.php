<?php
function f1() 
{
	echo "f1 called\n";
}

//switch warning on and test return value
var_dump($rao=assert_options(ASSERT_WARNING, 1));
$sa = "0 != 0";
var_dump($r2=assert($sa));
$sa = "0 == 0";
var_dump($r2=assert($sa));

//switch warning on and test return value
var_dump($rao=assert_options(ASSERT_WARNING, 0));
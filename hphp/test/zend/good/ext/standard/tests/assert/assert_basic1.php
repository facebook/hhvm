<?php
function f1() 
{
	echo "f1 called\n";
}

//String assert
$sa = "0 != 0";
var_dump($r2=assert($sa));
$sa = "0 == 0";
var_dump($r2=assert($sa));

//Non string assert 
var_dump($r2=assert(0));
var_dump($r2=assert(1));
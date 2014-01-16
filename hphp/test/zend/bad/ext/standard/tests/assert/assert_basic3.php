<?php
function f1() 
{
	echo "f1 called\n";
}

//bail out on error
var_dump($rao=assert_options(ASSERT_BAIL, 1));
$sa = "0 != 0";
var_dump($r2=assert($sa));
echo "If this is printed BAIL hasn't worked";
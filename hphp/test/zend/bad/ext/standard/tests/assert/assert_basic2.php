<?php
function f2() 
{
	echo "f2 called\n";
}

function f1() 
{
	echo "f1 called\n";
}
	
var_dump($o = assert_options(ASSERT_CALLBACK));
assert(0);
  
var_dump($o= assert_options(ASSERT_CALLBACK, "f2"));
var_dump($n= assert_options(ASSERT_CALLBACK));
assert(0);
?>
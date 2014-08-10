<?php
function a($file,$line,$myev)
{ 
	echo "assertion failed $line,\"$myev\"\n";
}

class a
{
	function assert($file,$line,$myev)
	{
		echo "class assertion failed $line,\"$myev\"\n";
	}
}

assert_options(ASSERT_ACTIVE,1);
assert_options(ASSERT_QUIET_EVAL,1);
assert_options(ASSERT_WARNING,0);

$a = 0;

assert_options(ASSERT_CALLBACK,"a");
assert('$a != 0');

assert_options(ASSERT_CALLBACK,array("a","assert"));
assert('$a != 0');

$obj = new a();
assert_options(ASSERT_CALLBACK,array(&$obj,"assert"));
assert('$a != 0');
?>

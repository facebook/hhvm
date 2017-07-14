<?php
error_reporting(E_ALL|E_STRICT);

class Foo 
{
    public function __toString()
	{
	    return __CLASS__;
	}
}

$num_repeats = 100000;

$start = memory_get_usage() / 1024;
for ($i=1;$i<$num_repeats;$i++) 
{
	$foo = new Foo();
	md5($foo);
}
$end = memory_get_usage() / 1024;

if ($start + 16 < $end) {
	echo 'FAIL';
} else {
	echo 'PASS';
}

?>

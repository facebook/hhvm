<?php

$a = new SplFixedArray(100);


function test(SplFixedArray &$arr) {
	print "ok\n";
}

try {
	test($a[]);
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

?>
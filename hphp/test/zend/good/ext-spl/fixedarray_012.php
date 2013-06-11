<?php

$a = new SplFixedArray(100);

try {
	$b = &$a[];
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

print "ok\n";

?>
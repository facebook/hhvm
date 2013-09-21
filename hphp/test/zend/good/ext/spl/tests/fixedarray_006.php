<?php

$b = 10000;
$a = new SplFixedArray($b);

try {
	for ($i = 0; $i < 100; $i++) {
		$a[] = new stdClass;
	}
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

print "ok\n";

?>
<?php

$a = new SplFixedArray(100);

function by_ref(&$ref) {}

try {
	by_ref(&$a[]);
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

print "ok\n";


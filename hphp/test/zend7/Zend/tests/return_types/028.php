<?php

function foo(): stdClass {
	$a = new stdClass;
	$b = [];
	return [$a, $b];
}

try {
	foo();
} catch (Error $e) {
	echo $e->getMessage(), " in ", $e->getFile(), " on line ", $e->getLine();
}

?>

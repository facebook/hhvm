<?php

$a = array(1,2,3);

$o = new stdclass;
$o->prop = "value";

try {
	var_dump($o + $a);
} catch (Error $e) {
	echo "\nException: " . $e->getMessage() . "\n";
}

$c = $o + $a;
var_dump($c);

echo "Done\n";
?>

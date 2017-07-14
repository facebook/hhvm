<?php

$a = array(1,2,3);

$o = new stdclass;
$o->prop = "value";

try {
	var_dump($a + $o);
} catch (Error $e) {
	echo "\nException: " . $e->getMessage() . "\n";
}

$c = $a + $o;
var_dump($c);

echo "Done\n";
?>

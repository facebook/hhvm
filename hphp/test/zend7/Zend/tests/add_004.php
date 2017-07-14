<?php

$a = array(1,2,3);

try {
	var_dump($a + 5);
} catch (Error $e) {
	echo "\nException: " . $e->getMessage() . "\n";
}

$c = $a + 5;
var_dump($c);

echo "Done\n";
?>

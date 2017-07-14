<?php

$a = array(1,2,3);
$b = array(1,2);

try {
	var_dump(~$b);
} catch (Error $e) {
	echo "\nException: " . $e->getMessage() . "\n";
}

$a = ~$b;
var_dump($a);

echo "Done\n";
?>

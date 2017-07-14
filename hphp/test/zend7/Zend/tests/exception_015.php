<?php
$s = "ABC";
try {
	$s[] = "D";
} catch (Error $e) {
	echo "\nException: " . $e->getMessage() . " in " , $e->getFile() . " on line " . $e->getLine() . "\n";
}

$s[] = "D";
?>

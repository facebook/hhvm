<?php
try {
	require("constant_expressions_exceptions.inc");
} catch (Error $e) {
	echo "\nException: " . $e->getMessage() . " in " , $e->getFile() . " on line " . $e->getLine() . "\n";
}
?>
DONE

<?php
class C {
	private $p = 0;
}

$x = new C;
try {
	var_dump($x->p);
} catch (Error $e) {
	echo "\nException: " . $e->getMessage() . " in " , $e->getFile() . " on line " . $e->getLine() . "\n";
}

var_dump($x->p);
?>

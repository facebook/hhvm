<?php

Class C {
	function __construct(&$x) {
		$x = "x.changed";
	}
}

$x = "x.original";
new C($x); // OK
var_dump($x);

$rc = new ReflectionClass('C');
$x = "x.original";
$rc->newInstance($x); // causes crash
var_dump($x);
$x = "x.original";
$rc->newInstanceArgs(array($x)); // causes crash	
var_dump($x);

echo "Done\n";
?>

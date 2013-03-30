<?php

class test { 
	static $foo = 1;
} 

$t = new test; 
$t->foo = 5;

$fp = fopen(__FILE__, 'r');

var_dump($t);

echo "Done\n";
?>
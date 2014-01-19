<?php
function valRef($x, &$y) {
	var_dump($x, $y);
	var_dump(func_num_args());
	$x = 'changed.x';
	$y = 'changed.y';
	var_dump(func_num_args());
}

function refVal(&$x, $y) {
	var_dump($x, $y);
	var_dump(func_num_args());
	$x = 'changed.x';
	$y = 'changed.y';
	var_dump(func_num_args());
}


echo "\n\n-- Val, Ref --\n";
$a = 'original.a';
valRef($a, $a);
var_dump($a);

echo "\n\n-- Ref, Val --\n";
$b = 'original.b';
refVal($b, $b);
var_dump($b);
?>
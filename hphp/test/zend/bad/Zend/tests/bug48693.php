<?php

$x = create_function('', 'return 1; }');
$y = create_function('', 'function a() { }; return 2;');
$z = create_function('', '{');
$w = create_function('', 'return 3;');

var_dump(
	$x,
	$y(),
	$z,
	$w(),
	$y != $z
);

?>
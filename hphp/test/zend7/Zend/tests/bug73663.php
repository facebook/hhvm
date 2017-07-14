<?php
function change(&$ref) {
	$ref = range(1, 10);
	return;
}

$func = function (&$ref) {
	return change($ref);
};

$array = [1];
var_dump(list($val) = $array); // NG: Invalid opcode

change(list($val) = $array);
var_dump($array);

$array = [1];

$func(list($val) = $array);
var_dump($array);
?>

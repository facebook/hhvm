<?php
function foo($value) {
	echo $value . " foo\n";
}

function bar($value) {
	echo $value . " bar\n";
}

$arr = array (1,2,3);
var_dump (array_walk_recursive ($arr, 'foo'));
var_dump (array_walk_recursive ($arr, 'bar'));

?>
<?php
function show_eq($x,$y) {
	echo "$x ". ($x==$y ? "==" : "!=") ." $y\n";
}

$value = 0x7FFFFFFF; #2147483647;
show_eq("$value", json_encode($value));
$value++;
show_eq("$value", json_encode($value));

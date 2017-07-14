<?php
$a = [1,2,3,4];
foreach($a as &$v) {
	echo "$v\n";
	array_shift($a);
}
var_dump($a);
?>

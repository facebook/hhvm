<?php
$a = [1,2,3,4,5];
foreach($a as &$v) {
	echo "$v\n";
	if ($v == 3) {
		array_walk($a, function (&$x) {$x+=10;});
	}
}
?>

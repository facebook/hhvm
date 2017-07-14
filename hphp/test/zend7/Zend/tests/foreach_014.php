<?php
$a = [1,2,3];
foreach($a as &$v) {
	echo "$v\n";
	if ($v == 2) {
		array_pop($a);
	}
}
?>

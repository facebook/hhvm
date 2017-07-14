<?php
$a = [1];
foreach($a as &$v) {
	echo "$v\n";
	$a[1]=2;
}
?>

<?php
$a = [1,2,3];
foreach($a as &$x) {
	foreach($a as &$y) {
		echo "$x-$y\n";
		$y++; 
	}
}
?>

<?php
$a = [0, 1, 2, 3, 4, 5, 6, 7];
unset($a[0], $a[1], $a[2], $a[3]);
foreach ($a as &$ref) {
	foreach ($a as &$ref2) {
		echo "$ref-$ref2\n";
		if ($ref == 5 && $ref2 == 6) {
			$a[42] = 8;
		}	
	}
}
?>

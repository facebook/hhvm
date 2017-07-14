<?php

function a() {
	$a = $b = $c = 2;
	foreach(range(1, 5) as $v) {
		yield $v;
	}
	return;
}

foreach (a(range(1, 3)) as $a) {
	var_dump($a);
}
?>

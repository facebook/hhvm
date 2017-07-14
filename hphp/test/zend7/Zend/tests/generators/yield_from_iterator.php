<?php
function g() {
	yield 1;
	yield from new ArrayIterator([2, 3, 4]);
	yield 5;
}

$g = g();
foreach ($g as $yielded) {
	var_dump($yielded);
}
?>

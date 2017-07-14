<?php
const FOO = [1];
const BAR = null;

function a(array $a = FOO) {
	var_dump($a);
}

function b(array $b = BAR) {
	var_dump($b);
}

b(null);
b([]);
b();
a([]);
a();
a(null);
?>

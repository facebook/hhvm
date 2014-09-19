<?php
const a = 10;

function t1($a = 1 + 1, $b = 1 << 2, $c = "foo" . "bar", $d = a * 10) {
	var_dump($a, $b, $c, $d);
}

t1();
?>

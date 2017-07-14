<?php
class foo {
	function __call($name, $args) {
		$a = $b = $c = $d = $e = $f = 1;
	}
}

function test($n, $x) {
//	var_dump($n);
	if ($n > 0) {
		$x->bug();
		test($n - 1, $x);
	}
}

test(3000, new foo());
echo "OK\n";
?>

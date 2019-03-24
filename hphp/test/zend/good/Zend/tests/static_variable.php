<?php
const bar = 2, baz = bar + 1;

function foo() {
	$a = 1 + 1;
	$b = [bar => 1 + 1, baz * 2 => 1 << 2];
	$c = [1 => bar, 3 => baz];
	var_dump($a, $b, $c);
}

foo();

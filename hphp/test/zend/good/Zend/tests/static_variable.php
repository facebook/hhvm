<?php
const bar = 2, baz = bar + 1;

function foo() {
static $a = 1 + 1;
static $b = [bar => 1 + 1, baz * 2 => 1 << 2];
static $c = [1 => bar, 3 => baz];
var_dump($a, $b, $c);
}

foo();
?>

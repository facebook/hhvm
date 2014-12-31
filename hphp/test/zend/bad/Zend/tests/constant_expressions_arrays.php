<?php
const a = [1,2,[3,[4]]];
const b = a[0];
const c = a[2][0];
const d = a[2];
const e = ["string" => [1]]["string"][0];

var_dump(b, c, e);

function test ($a = d[1][0]) {
	var_dump($a);
}

test();

class foo {
	const bar = [1][0];
}

var_dump(foo::bar);

var_dump(a, a[0], a[2], a[2][1], a[3]);

?>

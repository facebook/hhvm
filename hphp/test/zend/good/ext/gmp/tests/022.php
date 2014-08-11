<?php

$n = gmp_init("34293864345");
$n1 = gmp_init("23434293864345");

$a = array(
	array(123,45),
	array(4341,9734),
	array(23487,333),
	array(-234234,-123123),
	array(-100,-2234),
	array(345,"34587345"),
	array(345,"0"),
	array("345556456",345873),
	array("34545345556456","323432445873"),
	array($n, $n1),
	);

foreach ($a as $val) {
	$r = gmp_gcdext($val[0],$val[1]);
	var_dump(gmp_strval($r['g']));
	var_dump(gmp_strval($r['s']));
	var_dump(gmp_strval($r['t']));
}

var_dump(gmp_gcdext($val[0],array()));
var_dump(gmp_gcdext(array(),array()));
var_dump(gmp_gcdext(array(),array(),1));
var_dump(gmp_gcdext(array()));
var_dump(gmp_gcdext());

echo "Done\n";
?>

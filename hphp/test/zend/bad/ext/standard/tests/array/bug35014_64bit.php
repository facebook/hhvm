<?php
$tests = array(
	'foo',
	array(),
	array(0),
	array(3),
	array(3, 3),
	array(0.5, 2),
	array(99999999, 99999999),
	array(8.993, 7443241,988, sprintf("%u", -1)+0.44),
	array(2,sprintf("%u", -1)),
);

foreach ($tests as $v) {
	var_dump(array_product($v));
}
?>
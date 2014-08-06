<?php

$ary = array(
	0 => array(
		"a",
		1,
	),
	"a" => array(
		2,
		"b",
		3 => array(
			4,
			"c",
		),
		"3" => array(
			4,
			"c",
		),
	),
);

$it = new RecursiveTreeIterator(new RecursiveArrayIterator($ary));
foreach($it as $k => $v) {
	echo '[' . $it->key() . '] => ' . $it->getPrefix() . $it->getEntry() . $it->getPostfix() . "\n";
}
?>
===DONE===
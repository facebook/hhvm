<?php

$ary = array(
	0 => array(
		(binary) "binary",
		"abc2",
		1,
	),
	(binary) "binary" => array(
		2,
		"b",
		3 => array(
			4,
			"c",
		),
		"4abc" => array(
			4,
			"c",
		),
	),
);

$it = new RecursiveTreeIterator(new RecursiveArrayIterator($ary), 0);
foreach($it as $k => $v) {
	var_dump($v);
}
echo "\n----------------\n\n";
foreach($it as $k => $v) {
	var_dump($k);
}
echo "\n----------------\n\n";
echo "key, getEntry, current:\n";
foreach($it as $k => $v) {
	var_dump($it->key(), $it->getEntry(), $it->current());
}
?>
===DONE===
<?php

$arr = array(
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

$it = new RecursiveArrayIterator($arr);
$it = new RecursiveTreeIterator($it);

echo "----\n";
echo $it->getPostfix();
echo "\n\n";

echo "----\n";
$it->setPostfix("POSTFIX");
echo $it->getPostfix();
echo "\n\n";

echo "----\n";
foreach($it as $k => $v) {
	echo "[$k] => $v\n";
}

echo "----\n";
$it->setPostfix("");
echo $it->getPostfix();
echo "\n\n";

echo "----\n";
foreach($it as $k => $v) {
	echo "[$k] => $v\n";
}



?>
===DONE===
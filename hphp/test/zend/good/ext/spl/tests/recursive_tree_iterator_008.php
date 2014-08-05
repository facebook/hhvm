<?php

$ary = array(
	"a" => array("b"),
	"c" => array("d"),
);

$it = new RecursiveArrayIterator($ary);
$it = new RecursiveTreeIterator($it);
for($i = 0; $i < 6; ++$i) {
	$it->setPrefixPart($i, $i);
}
foreach($it as $k => $v) {
	echo "[$k] => $v\n";
}
try {
	$it->setPrefixPart(-1, "");
	$it->setPrefixPart(6, "");
} catch (OutOfRangeException $e) {
	echo "OutOfRangeException thrown\n";
}
try {
	$it->setPrefixPart(6, "");
} catch (OutOfRangeException $e) {
	echo "OutOfRangeException thrown\n";
}
?>
===DONE===
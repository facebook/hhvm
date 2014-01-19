<?php
$values = array("a", "aa", "aaa", "aaaa");
foreach ($values as $value) {
	$a = pack("H*", $value);
	$b = unpack("H*", $a);
	echo $value.": ";
	var_dump($b);
}
?>
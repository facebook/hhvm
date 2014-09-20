<?php

$types = array('SplDoublyLinkedList', 'SplObjectStorage', 'ArrayObject');

foreach ($types as $type) {
	// serialize an empty new object
	$exp = serialize(new $type());
	// hack to instanciate an object without constructor
	$str = sprintf('C:%d:"%s":0:{}', strlen($type), $type);
	$obj = unserialize($str);
	var_dump($obj);
	// serialize result
	$out = serialize($obj);
	// both should match
	var_dump($exp === $out);
}
?>
===DONE===

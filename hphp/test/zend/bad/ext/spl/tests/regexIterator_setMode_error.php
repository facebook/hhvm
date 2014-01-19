<?php

$array = array('foo', 'bar', 'baz');
$regexIterator = new RegexIterator(new ArrayIterator($array), "/f/");

var_dump($regexIterator->getMode());

try {
	$regexIterator->setMode(7);
} catch (InvalidArgumentException $e) {
	var_dump($e->getMessage());
	var_dump($e->getCode());
}

$regexIterator->setMode('foo');

?>
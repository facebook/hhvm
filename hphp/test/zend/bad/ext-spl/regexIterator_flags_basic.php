<?php

$array = array('foo', 'bar', 'baz');
$iterator = new ArrayIterator($array);
$regexIterator = new RegexIterator($iterator, "/f/", null, RegexIterator::USE_KEY);

var_dump($regexIterator->getFlags() === RegexIterator::USE_KEY);

// Test a change in flags, there's only one class constant so it has to be another int value
$regexIterator->setFlags(3);
var_dump($regexIterator->getFlags() === RegexIterator::USE_KEY);
$regexIterator->setFlags(RegexIterator::USE_KEY);
var_dump($regexIterator->getFlags() === RegexIterator::USE_KEY);

?>
<?php
$sample_array = array(1, 2, array(3, 4));

$iterator = new RecursiveIteratorIterator(new RecursiveArrayIterator($sample_array));

$iterator->next();
$iterator->next();
$iterator->next();
var_dump($iterator->getSubIterator(-1));
var_dump($iterator->getSubIterator(0)->getArrayCopy());
var_dump($iterator->getSubIterator(1)->getArrayCopy());
var_dump($iterator->getSubIterator(2));
?>
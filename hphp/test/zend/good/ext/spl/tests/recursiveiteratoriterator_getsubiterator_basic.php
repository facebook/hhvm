<?php
$sample_array = array(1, 2, array(3, 4));

$sub_iterator = new RecursiveArrayIterator($sample_array);
$not_sub_iterator = new RecursiveArrayIterator($sample_array);
$iterator = new RecursiveIteratorIterator($sub_iterator);

var_dump($iterator->getSubIterator() === $sub_iterator);
var_dump($iterator->getSubIterator() === $not_sub_iterator);
?>
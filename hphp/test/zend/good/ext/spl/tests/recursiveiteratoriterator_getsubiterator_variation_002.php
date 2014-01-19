<?php
$sample_array = array(1);

$iterator = new RecursiveIteratorIterator(new RecursiveArrayIterator($sample_array));

$iterator->next();
var_dump(is_null($iterator->getSubIterator()));
$iterator->next();
var_dump(is_null($iterator->getSubIterator()));
?>
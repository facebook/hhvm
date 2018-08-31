<?php


<<__EntryPoint>>
function main_array_iterator_current_return() {
$arr = new ArrayIterator(array(1));

$arr->next();
var_dump($arr->current());
var_dump($arr->key());
}

<?php

$array = new ArrayObject();
$array->offsetSet('key', 'value');
var_dump($array->offsetExists('key'));
var_dump($array->offsetExists('nokey'));

?>
===DONE===
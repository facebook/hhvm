<?php
$e = new Exception();

$ref = new ReflectionProperty($e, 'trace');
$ref->setAccessible(TRUE);

echo "Array of NULL:\n";
$ref->setValue($e, array(NULL));

var_dump($e->getTraceAsString());

echo "\nArray of empty array:\n";
$ref->setValue($e, array(array()));
var_dump($e->getTraceAsString());

echo "\nArray of array of NULL values:\n";
$ref->setValue($e, array(array(
    'file'  => NULL,
    'line'  => NULL,
    'class' => NULL,
    'type'  => NULL,
    'function' => NULL,
    'args'  => NULL
)));
var_dump($e->getTraceAsString());
?>
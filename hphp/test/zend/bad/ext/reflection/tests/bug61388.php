<?php
$x = new ArrayObject();
$x[0] = 'test string 2';
$x['test'] = 'test string 3';
$reflObj = new ReflectionObject($x);
print_r($reflObj->getProperties(ReflectionProperty::IS_PUBLIC));

$x = (object)array("a", "oo" => "b");
$reflObj = new ReflectionObject($x);
print_r($reflObj->getProperties(ReflectionProperty::IS_PUBLIC));

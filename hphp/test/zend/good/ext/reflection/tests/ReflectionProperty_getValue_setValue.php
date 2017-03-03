<?php

class PropertyTest
{
    public $prop = 'initial value';
}

$obj = new PropertyTest();

$reflection = new ReflectionObject($obj);
$property = $reflection->getProperty('prop');

var_dump($property->getValue($obj) == 'initial value');

$property->setValue($obj, 'new value');

var_dump($property->getValue($obj) == 'new value');

?>

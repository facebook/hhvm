<?php
class foo
{
    public $bar;
}

error_reporting(-1);

$foo  = new foo;
$obj  = new ReflectionObject($foo);
$prop = $obj->getProperty('bar');

var_dump($prop->getValue($foo));
unset($foo->bar);
var_dump($prop->getValue($foo));

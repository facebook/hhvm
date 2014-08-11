<?php

$closure1   = function($val){ return $val; };
$closure2   = function($val){ return $val; };

$reflection_class   = new ReflectionClass($closure1);
$reflection_method  = $reflection_class->getMethod('__invoke');

$arguments1         = array('hello');
$arguments2         = array('world');

var_dump($reflection_method->invokeArgs($closure1, $arguments1));
var_dump($reflection_method->invokeArgs($closure2, $arguments2));

?>

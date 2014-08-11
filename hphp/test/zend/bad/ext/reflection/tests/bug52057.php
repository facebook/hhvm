<?php

$closure = function($a) { echo $a; };

$reflection = new ReflectionClass('closure');
var_dump($reflection->hasMethod('__invoke')); // true

$reflection = new ReflectionClass($closure);
var_dump($reflection->hasMethod('__invoke')); // true

$reflection = new ReflectionObject($closure);
var_dump($reflection->hasMethod('__invoke')); // true

$reflection = new ReflectionClass('closure');
var_dump($h = $reflection->getMethod('__invoke')); // true
var_dump($h->class.'::'.$h->getName());

$reflection = new ReflectionClass($closure);
var_dump($h = $reflection->getMethod('__invoke')); // true
var_dump($h->class.'::'.$h->getName());

$reflection = new ReflectionObject($closure);
var_dump($h = $reflection->getMethod('__invoke')); // true
var_dump($h->class.'::'.$h->getName());

?>

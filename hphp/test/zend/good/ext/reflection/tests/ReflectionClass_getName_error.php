<?php

$r1 = new ReflectionClass("stdClass");

var_dump($r1->getName('X'));
var_dump($r1->getName('X', true));
?> 

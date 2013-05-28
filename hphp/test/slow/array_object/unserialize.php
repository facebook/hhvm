<?php

$a = new ArrayObject(array(1,2,3));
$b = $a->serialize();
$c = new ArrayObject;
$d = $c->unserialize($b);

var_dump($a);
var_dump($c);
var_dump($d);
var_dump($a == $c);

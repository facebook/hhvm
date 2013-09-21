<?php

$fa = new SplFixedArray(2);
$fa[0] = 'Hello';
$fa[1] = 'World';
$fa->setSize(3);
$fa[2] = '!';
var_dump($fa);
$fa->setSize(2);
var_dump($fa);
var_dump($fa->getSize());


?>
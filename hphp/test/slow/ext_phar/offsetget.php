<?php

$x = new Phar(__DIR__.'/basic.phar');
$obj = $x->offsetGet('index.php');
var_dump(get_class($obj));
var_dump($obj->getFilename());
var_dump($obj->getSize());
var_dump($obj->getExtension());

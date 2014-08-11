<?php
class C {}
$rc = new ReflectionClass("C");
var_dump($rc->isFinal('X'));
var_dump($rc->isInterface(null));
var_dump($rc->isAbstract(true));
var_dump($rc->getModifiers(array(1,2,3)));

?>

<?php
class C {}
$rc = new ReflectionObject(new C);
var_dump($rc->getConstructor(null));
var_dump($rc->getConstructor('X'));
var_dump($rc->getConstructor(true));
var_dump($rc->getConstructor(array(1,2,3)));
?>

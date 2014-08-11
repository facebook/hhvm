<?php
interface I {}
class C implements I {}
$rc = new ReflectionClass('C');
var_dump($rc->getInterfaces(null));
var_dump($rc->getInterfaces('X'));
var_dump($rc->getInterfaces(true));
var_dump($rc->getInterfaces(array(1,2,3)));
?>

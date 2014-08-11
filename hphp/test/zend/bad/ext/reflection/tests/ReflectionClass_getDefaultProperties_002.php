<?php
interface I {}
class C implements I {}
$rc = new ReflectionClass('C');
var_dump($rc->getDefaultProperties(null));
var_dump($rc->getDefaultProperties('X'));
var_dump($rc->getDefaultProperties(true));
var_dump($rc->getDefaultProperties(array(1,2,3)));
var_dump($rc->getStaticProperties(null));
var_dump($rc->getStaticProperties('X'));
var_dump($rc->getStaticProperties(true));
var_dump($rc->getStaticProperties(array(1,2,3)));

?>

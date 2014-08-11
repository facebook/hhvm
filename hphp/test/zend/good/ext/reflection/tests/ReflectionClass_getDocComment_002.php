<?php
class C {}
$rc = new ReflectionClass('C');
var_dump($rc->getDocComment(null));
var_dump($rc->getDocComment('X'));
var_dump($rc->getDocComment(true));
var_dump($rc->getDocComment(array(1,2,3)));
?>

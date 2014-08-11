<?php
class C { function f() {} }
$rc = new ReflectionMethod('C::f');
var_dump($rc->getDocComment(null));
var_dump($rc->getDocComment('X'));
?>

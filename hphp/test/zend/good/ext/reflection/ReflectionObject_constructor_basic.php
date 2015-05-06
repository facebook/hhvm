<?php
$r1 = new ReflectionObject(new stdClass);
var_dump($r1);

class C { }
$myInstance = new C;
$r2 = new ReflectionObject($myInstance);
var_dump($r2);

$r3 = new ReflectionObject($r2);
var_dump($r3);
?>

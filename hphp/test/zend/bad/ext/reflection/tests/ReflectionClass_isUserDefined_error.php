<?php
$r1 = new ReflectionClass("stdClass");
var_dump($r1->isUserDefined('X'));
var_dump($r1->isUserDefined('X', true));
?>

<?php
$r1 = new ReflectionClass("stdClass");
var_dump($r1->isInternal('X'));
var_dump($r1->isInternal('X', true));
?>

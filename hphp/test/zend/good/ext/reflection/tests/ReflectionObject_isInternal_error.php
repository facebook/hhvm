<?php

$r1 = new ReflectionObject(new stdClass);
var_dump($r1->isInternal('X'));
var_dump($r1->isInternal('X', true));
?>

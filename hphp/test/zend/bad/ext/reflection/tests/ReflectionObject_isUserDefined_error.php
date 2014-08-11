<?php
$r1 = new ReflectionObject(new stdClass);

var_dump($r1->isUserDefined('X'));
var_dump($r1->isUserDefined('X', true));
?>

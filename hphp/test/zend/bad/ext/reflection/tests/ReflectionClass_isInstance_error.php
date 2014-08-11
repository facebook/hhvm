<?php
class X {}

$rc = new ReflectionClass("X");
$instance = new X;

var_dump($rc->isInstance());
var_dump($rc->isInstance($instance, $instance));
var_dump($rc->isInstance(1));
var_dump($rc->isInstance(1.5));
var_dump($rc->isInstance(true));
var_dump($rc->isInstance('X'));
var_dump($rc->isInstance(null));

?>

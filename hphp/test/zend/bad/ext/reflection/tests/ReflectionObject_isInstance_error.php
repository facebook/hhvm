<?php
class X {}
$instance = new X;
$ro = new ReflectionObject(new X);

var_dump($ro->isInstance());
var_dump($ro->isInstance($instance, $instance));
var_dump($ro->isInstance(1));
var_dump($ro->isInstance(1.5));
var_dump($ro->isInstance(true));
var_dump($ro->isInstance('X'));
var_dump($ro->isInstance(null));

?>

<?php
class A {}

$class = new ReflectionClass('A');
$instance = $class->newInstanceArgs();
var_dump($instance);

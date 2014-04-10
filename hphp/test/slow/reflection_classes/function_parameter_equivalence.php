<?php
function a($a, $b, $c) {}
function b($a, $b, $c) {}
function c($a, $c, $c) {}

$reflected_a = new ReflectionFunction('a');
$reflected_b = new ReflectionFunction('b');
$reflected_c = new ReflectionFunction('c');
var_dump($reflected_a->getParameters() == $reflected_b->getParameters());
var_dump($reflected_a->getParameters() == $reflected_c->getParameters());


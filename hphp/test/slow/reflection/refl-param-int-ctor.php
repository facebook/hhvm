<?php

class C {
  public function m($a, $b) { }
}

// same as doing 'a' instead of 0
$refl = new ReflectionParameter(array('C', 'm'), 0);

var_dump($refl->getDeclaringClass()->getName());
var_dump($refl->getDeclaringFunction()->getName());
var_dump($refl->getName());

$refl = new ReflectionParameter(array('C', 'm'), 1);

var_dump($refl->getDeclaringClass()->getName());
var_dump($refl->getDeclaringFunction()->getName());
var_dump($refl->getName());

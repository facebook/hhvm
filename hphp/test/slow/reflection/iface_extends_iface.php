<?php

interface A {
  public function get($x);
}

interface B extends A { }

$refl = new ReflectionMethod('A', 'get');
var_dump($refl->getDeclaringClass()->getName());
var_dump($refl->getName());
var_dump($refl->getParameters()[0]->getName());
var_dump(count($refl->getParameters()));
$refl = new ReflectionMethod('B', 'get');
var_dump($refl->getDeclaringClass()->getName());
var_dump($refl->getName());
var_dump($refl->getParameters()[0]->getName());
var_dump(count($refl->getParameters()));

<?php

abstract class Test {
  abstract public function myMethod();
}

$refl = new ReflectionClass('test');
var_dump($refl->getMethods(ReflectionMethod::IS_ABSTRACT)[0]->name);

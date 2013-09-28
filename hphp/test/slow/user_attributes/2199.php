<?php

<<A(1)>>
function f() {
}
$rf = new ReflectionFunction('f');
var_dump($rf->getAttribute('A'));
var_dump($rf->getAttribute('B'));
var_dump($rf->getAttributes());
var_dump($rf->getAttributeRecursive('A'));
var_dump($rf->getAttributeRecursive('B'));
var_dump($rf->getAttributesRecursive());

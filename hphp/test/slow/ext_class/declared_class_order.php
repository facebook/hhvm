<?php
class A {
}

$classes = get_declared_classes();
var_dump(count($classes) > 1);
var_dump($classes[count($classes) - 1]);

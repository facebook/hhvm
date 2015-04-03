<?php

class Foo {}

class Bar extends Foo {}

$rc1 = new ReflectionClass("Bar");
var_dump($rc1->getParentClass());
?>


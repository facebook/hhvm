<?php

$a = new ArrayObject(["hello"]);
$ref = new ReflectionClass($a);

$ref->getProperty("storage")->setAccessible(true);
unset($a->storage);

var_dump((array) $a);

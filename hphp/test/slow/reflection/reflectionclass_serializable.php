<?php

class Foo {
}

$rc = new ReflectionClass(Foo::class);
$serialized = serialize($rc);
var_dump($serialized);
var_dump(unserialize($serialized)->getName());

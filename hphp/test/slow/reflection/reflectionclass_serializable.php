<?php

class Foo {
}

$rc = new ReflectionClass(Foo::class);
$serialized = serialize($rc);
var_dump(json_encode($serialized));
var_dump(unserialize($serialized)->getName());

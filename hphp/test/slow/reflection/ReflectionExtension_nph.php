<?php
// Test native property handling with ReflectionExtension

$x = new ReflectionExtension("mysqli");
var_dump($x->getName());
var_dump($x->name);
// This should fatal since $name is technically a read only prop
$x->name = "Bad";
// We should never get here
var_dump($x->name);

<?php
class D {
    // empty class
}

class A extends D {
    // empty class
}

$reflection = new \ReflectionClass( '\A' );

var_dump(
    $reflection->isAbstract(),
    $reflection->name,
    $reflection->isSubclassOf( '\D' )
);

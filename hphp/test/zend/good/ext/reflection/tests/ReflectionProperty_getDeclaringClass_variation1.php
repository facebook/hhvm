<?php

class A {
    public $prop;
}

class B extends A {
}

$propInfo = new ReflectionProperty('B', 'prop');
var_dump($propInfo->getDeclaringClass());

echo "Wrong number of params:\n";
$propInfo->getDeclaringClass(1);

?> 

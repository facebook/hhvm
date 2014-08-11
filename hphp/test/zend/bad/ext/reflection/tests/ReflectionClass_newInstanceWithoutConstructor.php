<?php
class Foo
{
    public function __construct()
    {
        print __METHOD__;
    }
}

$class = new ReflectionClass('Foo');
var_dump($class->newInstanceWithoutConstructor());

$class = new ReflectionClass('StdClass');
var_dump($class->newInstanceWithoutConstructor());

$class = new ReflectionClass('DateTime');
var_dump($class->newInstanceWithoutConstructor());

<?php

class Foo
{
	protected static $fooStatic = 'foo';
	protected $foo = 'foo';
}

$class = new ReflectionClass('Foo');

var_dump($class->getDefaultProperties());

echo "Done\n";
?>

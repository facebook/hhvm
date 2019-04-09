<?php
class Foo
{
	const A = 1;
	const B = self::A;
}

$rc = new ReflectionClass('Foo');
print_r($rc->getConstants());

class Foo2
{
        const A = 1;
        const B = self::A;
}

$rc = new ReflectionClass('Foo2');
print_r($rc->getConstant('B'));

<?php

define("CONST_TEST_1", "const1");

function ReflectionParameterTest($test1=array(), $test2 = CONST_TEST_1) {
	echo $test;
}
$reflect = new ReflectionFunction('ReflectionParameterTest');
foreach($reflect->getParameters() as $param) {
	if($param->getName() == 'test1') {
		var_dump($param->isDefaultValueConstant());
	}
	if($param->getName() == 'test2') {
		var_dump($param->isDefaultValueConstant());
	}
	if($param->isDefaultValueAvailable() && $param->isDefaultValueConstant()) {
		var_dump($param->getDefaultValueConstantName());
	}
}

class Foo2 {
	const bar = 'Foo2::bar';
}

class Foo {
	const bar = 'Foo::bar';

	public function baz($param1 = self::bar, $param2=Foo2::bar, $param3=CONST_TEST_1) {
	}
}

$method = new ReflectionMethod('Foo', 'baz');
$params = $method->getParameters();

foreach ($params as $param) {
    if ($param->isDefaultValueConstant()) {
        var_dump($param->getDefaultValueConstantName());
    }
}
?>
==DONE==

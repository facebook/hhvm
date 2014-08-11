<?php

class TestClass
{
	static function test()
	{
		static $enabled = true;
	}
}

$class = new ReflectionClass('TestClass');
foreach ($class->getMethods() as $method)
{
	var_dump($method->getName());
	$arr_static_vars[] = $method->getStaticVariables();
}

var_dump($arr_static_vars);

?>
===DONE===

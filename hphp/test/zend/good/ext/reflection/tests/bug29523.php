<?php

class TestClass
{
}

function optionalTest(TestClass $a, TestClass $b, $c = 3)
{
}

$function = new ReflectionFunction('optionalTest'); 
$numberOfNotOptionalParameters = 0;
$numberOfOptionalParameters = 0;
foreach($function->getParameters() as $parameter)
{
	var_dump($parameter->isOptional());
	if ($parameter->isOptional())
	{
		++$numberOfOptionalParameters;
	}
	else
	{
		++$numberOfNotOptionalParameters;
	}
}
var_dump($function->getNumberOfRequiredParameters());
var_dump($numberOfNotOptionalParameters);

?>

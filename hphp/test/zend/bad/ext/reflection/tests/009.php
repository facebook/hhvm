<?php

/**
hoho
*/
function test ($a, $b = 1, $c = "") {
	static $var = 1;
}

$func = new ReflectionFunction("test");

var_dump($func->export("test"));
echo "--getName--\n";
var_dump($func->getName());
echo "--isInternal--\n";
var_dump($func->isInternal());
echo "--isUserDefined--\n";
var_dump($func->isUserDefined());
echo "--getFilename--\n";
var_dump($func->getFilename());
echo "--getStartline--\n";
var_dump($func->getStartline());
echo "--getEndline--\n";
var_dump($func->getEndline());
echo "--getDocComment--\n";
var_dump($func->getDocComment());
echo "--getStaticVariables--\n";
var_dump($func->getStaticVariables());
echo "--invoke--\n";
var_dump($func->invoke(array(1,2,3)));
echo "--invokeArgs--\n";
var_dump($func->invokeArgs(array(1,2,3)));
echo "--returnsReference--\n";
var_dump($func->returnsReference());
echo "--getParameters--\n";
var_dump($func->getParameters());
echo "--getNumberOfParameters--\n";
var_dump($func->getNumberOfParameters());
echo "--getNumberOfRequiredParameters--\n";
var_dump($func->getNumberOfRequiredParameters());

echo "Done\n";

?>

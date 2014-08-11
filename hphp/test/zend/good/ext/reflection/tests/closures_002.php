<?php

class Test {
	function __invoke($a, $b = 0) { }
}

$rm = new ReflectionMethod(new Test, '__invoke');
var_dump($rm->getName());
var_dump($rm->getNumberOfParameters());
var_dump($rm->getNumberOfRequiredParameters());

$rp = new ReflectionParameter(array(new Test, '__invoke'), 0);
var_dump($rp->isOptional());

$rp = new ReflectionParameter(array(new Test, '__invoke'), 1);
var_dump($rp->isOptional());

?>
===DONE===

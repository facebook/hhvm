<?php

class Test {
	function func($x, $y = NULL){
	}
}


$f = new ReflectionMethod('Test', 'func');
var_dump($f->getNumberOfParameters());
var_dump($f->getNumberOfRequiredParameters());

$p = new ReflectionParameter(array('Test', 'func'), 'x');
var_dump($p->isOptional());

$p = new ReflectionParameter(array('Test', 'func'), 'y');
var_dump($p->isOptional());

try {
	$p = new ReflectionParameter(array('Test', 'func'), 'z');
	var_dump($p->isOptional());
}
catch (Exception $e) {
	var_dump($e->getMessage());
}

?>
===DONE===

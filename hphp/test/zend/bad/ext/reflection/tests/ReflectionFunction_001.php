<?php

/**
 * my doc comment
 */
function foo () {
	static $c;
	static $a = 1;
	static $b = "hello";
	$d = 5;
}

/***
 * not a doc comment
 */
function bar () {}


function dumpFuncInfo($name) {
	$funcInfo = new ReflectionFunction($name);
	var_dump($funcInfo->getName());
	var_dump($funcInfo->isInternal());
	var_dump($funcInfo->isUserDefined());
	var_dump($funcInfo->getStartLine());
	var_dump($funcInfo->getEndLine());
	var_dump($funcInfo->getStaticVariables());
}

dumpFuncInfo('foo');
dumpFuncInfo('bar');
dumpFuncInfo('extract');

?>

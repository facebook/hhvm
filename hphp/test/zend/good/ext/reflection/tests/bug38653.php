<?php

class foo {
	    const cons = 10;
	    const cons1 = "";
	    const cons2 = "test";
}

class bar extends foo {
}

$foo = new ReflectionClass("foo");
var_dump($foo->getConstant("cons"));
var_dump($foo->getConstant("cons1"));
var_dump($foo->getConstant("cons2"));
var_dump($foo->getConstant("no such const"));

echo "Done\n";
?>

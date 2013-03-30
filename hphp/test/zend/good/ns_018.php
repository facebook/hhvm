<?php
namespace test;

function foo() {
	return __FUNCTION__;
}

$x = __NAMESPACE__ . "\\foo"; 
echo $x(),"\n";
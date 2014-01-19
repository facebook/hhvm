<?php
namespace test\ns1;

function strlen($x) {
	return __FUNCTION__;
}

$x = "strlen";
echo $x("Hello"),"\n";
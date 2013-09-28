<?php
namespace test\ns1;

function strlen($x) {
	return __FUNCTION__;
}

$x = "test\\ns1\\strlen";
echo $x("Hello"),"\n";
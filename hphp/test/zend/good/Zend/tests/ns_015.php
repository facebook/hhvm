<?php
namespace test\ns1;

function strlen($x) {
	return __FUNCTION__;
}

echo \strlen("Hello"),"\n";
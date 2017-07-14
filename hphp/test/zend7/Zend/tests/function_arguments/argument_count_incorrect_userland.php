<?php
try {
	function foo($bar) { }
	foo();
} catch (\Error $e) {
	echo get_class($e) . PHP_EOL;
	echo $e->getMessage() . PHP_EOL;
}

try {
	function bar($foo, $bar) { }
	bar(1);
} catch (\Error $e) {
	echo get_class($e) . PHP_EOL;
	echo $e->getMessage() . PHP_EOL;
}

function bat(int $foo, string $bar) { }

try {
	bat(123);
} catch (\Error $e) {
	echo get_class($e) . PHP_EOL;
	echo $e->getMessage() . PHP_EOL;
}

try {
	bat("123");
} catch (\Error $e) {
	echo get_class($e) . PHP_EOL;
	echo $e->getMessage() . PHP_EOL;
}

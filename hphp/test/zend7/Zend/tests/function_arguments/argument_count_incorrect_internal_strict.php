<?php
declare(strict_types=1);
try {
	substr("foo");
} catch (\Error $e) {
	echo get_class($e) . PHP_EOL;
	echo $e->getMessage() . PHP_EOL;
}

array_diff([]);

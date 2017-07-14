<?php
abstract class C {
	abstract static function foo();
}

function foo(callable $x) {
}

try {
	C::foo();
} catch (Error $e) {
	echo "\nException: " . $e->getMessage() . " in " , $e->getFile() . " on line " . $e->getLine() . "\n";
}

try {
	foo("C::foo");
} catch (Error $e) {
	echo "\n";
	do {
		echo "Exception: " . $e->getMessage() . "\n";
		$e = $e->getPrevious();
	} while ($e instanceof Error);
}

C::foo();
?>

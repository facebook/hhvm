<?php
	error_reporting(E_ALL & !E_STRICT);

	class A {
		function hello() {
			echo "Hello World\n";
		}
	}
	$y = ['hello'];
	A::$y[0]();
echo "===DONE===\n";

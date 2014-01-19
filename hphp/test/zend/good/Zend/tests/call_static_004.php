<?php

class foo {
	static function __callstatic($a, $b) {
		var_dump($a);
	}
}

foo::AaA();

$a = 1;
foo::$a();

?>
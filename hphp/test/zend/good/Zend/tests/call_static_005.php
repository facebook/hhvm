<?php

class foo {
	static function __callstatic($a, $b) {
		var_dump($a);
	}
}

$a = 'foo::';
$a();

?>
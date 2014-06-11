<?php

class Foo {
	private $bar = 'baz';
}

echo json_encode([[], (object)[], new Foo], JSON_PRETTY_PRINT), "\n";

?>

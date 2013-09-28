<?php 

error_reporting(E_ALL|E_STRICT);

class Foo { }

try {
	throw new Foo();
} catch (Foo $e) {
	var_dump($e);
}

?>
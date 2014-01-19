<?php

class myClass {
	public static function __callStatic($method, $args) {
		throw new Exception("Missing static method '$method'\n");
	}
	public function __call($method, $args) {
		throw new Exception("Missing method '$method'\n");
	}
}

function thrower() {
	myClass::ThrowException();
}
function thrower2() {
	$x = new myClass;
	$x->foo();
}

try {
	thrower();
} catch(Exception $e) {
	print $e->getMessage();
	print_r($e->getTrace());
}

try {
	thrower2();
} catch (Exception $e) {
	print $e->getMessage();
	print_r($e->getTrace());
}

?>
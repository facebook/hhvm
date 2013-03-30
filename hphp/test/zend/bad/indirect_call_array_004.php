<?php

class foo {
	static public function abc() {
		throw new Exception('foo');
	}
	public function __call($a, $b) {
		printf("From %s:\n", __METHOD__);
		throw new Exception($a);
	}
	static public function __callStatic($a, $b) {
		printf("From %s:\n", __METHOD__);
		throw new Exception($a);
	}
}


$arr = array('foo', 'abc');

try {
	$arr();
}
catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

$arr = array('foo', '123');

try {
	$arr();
}
catch (Exception $e) {
	echo $e->getMessage(), "\n";
}


echo "------\n";

$foo = new foo;
$arr = array($foo, 'abc');

try {
	$arr();
}
catch (Exception $e) {
	echo $e->getMessage(), "\n";
}


$foo = new foo;
$arr = array($foo, '123');

try {
	$arr();
}
catch (Exception $e) {
	echo $e->getMessage(), "\n";
}

?>
<?php
function foo1() {
	try {
		throw new Exception("not catch");
		return true;
	} finally {
		try {
			throw new Exception("catched");
		} catch (Exception $e) {
		}
	}
}
try {
	$foo = foo1();
	var_dump($foo);
} catch (Exception $e) {
	do {
		var_dump($e->getMessage());
	} while ($e = $e->getPrevious());
}

function foo2() {
	try  {
		try {
			throw new Exception("catched");
			return true;
		} finally {
			try {
				throw new Exception("catched");
			} catch (Exception $e) {
			}
		}
	} catch (Exception $e) {
	}
}

$foo = foo2();
var_dump($foo);

function foo3() {
	try {
		throw new Exception("not catched");
		return true;
	} finally {
		try {
			throw new NotExists();
		} catch (Exception $e) {
		}
	}
}

$bar = foo3();
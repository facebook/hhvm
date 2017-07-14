<?php
try {
	$e = new Exception("Foo");
	try {
		throw  new Exception("Bar", 0, $e);
	} finally {
		throw $e;
	}
} catch (Exception $e) {
	var_dump((string)$e);
}

try {
	$e = new Exception("Foo");
	try {
		throw new Exception("Bar", 0, $e);
	} finally {
		throw new Exception("Dummy", 0, $e);
	}
} catch (Exception $e) {
	var_dump((string)$e);
}
?>

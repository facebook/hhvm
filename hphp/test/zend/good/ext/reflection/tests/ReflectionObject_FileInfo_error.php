<?php
Class C { }

$rc = new ReflectionObject(new C);
$methods = array("getFileName", "getStartLine", "getEndLine");

foreach ($methods as $method) {
	var_dump($rc->$method());
	var_dump($rc->$method(null));
	var_dump($rc->$method('X', 0));	
}
?>

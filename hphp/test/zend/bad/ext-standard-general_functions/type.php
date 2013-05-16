<?php

function foo($errno, $errstr, $errfile, $errline) {
	var_dump($errstr);
}

set_error_handler("foo");

$fp = fopen(__FILE__, "r");
fclose($fp);
$fp1 = fopen(__FILE__, "r");

$var1 = "another string";
$var2 = array(2,3,4);

$array = array(
	array(1,2,3),
	$var1,
	$var2,
	1,
	2.0,
	NULL,
	false,
	"some string",
	$fp,
	$fp1,
	new stdclass,
);

$types = array(
	"null",
	"integer",
	"double",
	"boolean",
	"resource",
	"array",
	"object",
	"string"
	);

foreach ($array as $var) {
	var_dump(gettype($var));
}

foreach ($types as $type) {
	foreach ($array as $var) {
		var_dump(settype($var, $type));
		var_dump($var);
	}
}

echo "Done\n";
?>
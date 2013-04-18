<?php
date_default_timezone_set('UTC');
class _d extends DateTime {
	function __construct() {
	}
}
class _t extends DateTimeZone {
	function __construct() {
	}
}

$d = new DateTime;
var_dump($d->format("Y-m-d H:i:s"));

$d = new _d;
var_dump($d->format("Y-m-d H:i:s"));

try {
	new DateTime("1am todax");
} catch (Exception $e) {
	echo $e->getMessage(),"\n";
}

$t = new DateTimeZone("UTC");
var_dump($t->getName());

$t = new _t;
var_dump($t->getName());

try {
	new DateTimeZone("GottaFindThisOne");
} catch (Exception $e) {
	echo $e->getMessage(),"\n";
}

echo "DONE\n";
?>
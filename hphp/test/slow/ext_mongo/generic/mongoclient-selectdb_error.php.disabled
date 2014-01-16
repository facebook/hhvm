<?php
require_once __DIR__."/../utils/server.inc";
$m = mongo_standalone();
$db = $m->selectDB();
echo is_object($db) ? '1' : '0', "\n";

$db = $m->selectDB(array('test'));
echo is_object($db) ? '1' : '0', "\n";

try {
	$db = $m->selectDB(NULL);
} catch(Exception $e) {
	echo $e->getMessage() . ".\n";
}

$db = $m->selectDB(new stdClass);
echo is_object($db) ? '1' : '0', "\n";
?>
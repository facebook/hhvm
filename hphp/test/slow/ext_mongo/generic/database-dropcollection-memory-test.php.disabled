<?php
require_once __DIR__."/../utils/server.inc";
$a = mongo_standalone();
$d = $a->selectDb("phpunit");
$ns = $d->selectCollection('system.namespaces');

// drop the collection 100 times
$u = memory_get_usage(true);
for ($i = 0; $i < 100; $i++) {
	$d->dropCollection($d->dropcoltest);
}
var_dump($u - memory_get_usage(true));

// drop the collection 100 times
$u = memory_get_usage(true);
for ($i = 0; $i < 100; $i++) {
	$d->dropCollection('dropcoltest');
}
var_dump($u - memory_get_usage(true));

?>
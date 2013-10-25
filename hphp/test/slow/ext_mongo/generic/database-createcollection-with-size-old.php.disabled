<?php
require_once __DIR__."/../utils/server.inc";
$a = mongo_standalone();
$d = $a->selectDb("phpunit");
$ns = $d->selectCollection('system.namespaces');

// cleanup
$d->dropCollection('create-col1');
var_dump($ns->findOne(array('name' => 'phpunit.create-col1')));

// create
// * even though we're only setting this to 100, it allocates 1 extent, so we
//   can fit 4096, not 100, bytes of data in the collection.

$c = $d->createCollection('create-col1', true, 100);
var_dump($ns->findOne(array('name' => 'phpunit.create-col1')));

// test cap
for ($i = 0; $i < 100; $i++) {
	$c->insert(array('x' => $i));
}
var_dump($c->count());
var_dump($c->count() < 100);
?>
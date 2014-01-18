<?php
require_once __DIR__."/../utils/server.inc";
$a = mongo_standalone();
$d = $a->selectDb("phpunit");
$ns = $d->selectCollection('system.namespaces');

// cleanup
$d->dropCollection('create-col1');
$retval = $ns->findOne(array('name' => 'phpunit.create-col1'));
var_dump($retval);

// create
$d->createCollection();
$d->createCollection(array());
$d->createCollection(fopen(__FILE__, 'r'));
?>
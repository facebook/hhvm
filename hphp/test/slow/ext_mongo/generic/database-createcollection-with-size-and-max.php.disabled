<?php
require_once __DIR__."/../utils/server.inc";

$a = mongo_standalone();
$d = $a->selectDb("phpunit");

// cleanup
$d->selectCollection("createcol1")->drop();

$ns = $d->selectCollection('system.namespaces');
var_dump($ns->findOne(array('name' => 'phpunit.createcol1')));

// create
$c = $d->createCollection('createcol1', array('capped' => true, 'size' => 1000, 'max' => 5));
$retval = $ns->findOne(array('name' => 'phpunit.createcol1'));
var_dump($retval["name"]);

// test cap
for ($i = 0; $i < 10; $i++) {
    $c->insert(array('x' => $i), array("safe" => true));
}
foreach($c->find() as $res) {
    var_dump($res["x"]);
}
var_dump($c->count());
?>
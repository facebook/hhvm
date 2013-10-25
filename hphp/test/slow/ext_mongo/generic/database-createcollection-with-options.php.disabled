<?php
require_once __DIR__."/../utils/server.inc";
$dsn = MongoShellServer::getStandaloneInfo();

$a = new MongoClient($dsn);
$d = $a->selectDb("phpunit");
$ns = $d->selectCollection('system.namespaces');

// cleanup
$d->dropCollection('create-col1');
var_dump($ns->findOne(array('name' => 'phpunit.create-col1')));

// create
// * even though we're only setting this to 100, it allocates 1 extent, so we
//   can fit 4096, not 100, bytes of data in the collection.

$c = $d->createCollection('create-col1', array('size' => 100, 'capped' => true, 'autoIndexId' => true, 'max' => 5));
var_dump($ns->findOne(array('name' => 'phpunit.create-col1')));

// check indexes
var_dump($c->getIndexInfo());

// test cap
for ($i = 0; $i < 10; $i++) {
    $c->insert(array('x' => $i), array("safe" => true));
}
foreach($c->find() as $res) {
    var_dump($res["x"]);
}
var_dump($c->count());
?>
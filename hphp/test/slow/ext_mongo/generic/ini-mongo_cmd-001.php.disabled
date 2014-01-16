<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'bar');
$coll->drop();

ini_set('mongo.cmd', '!');
$coll->insert(array('_id' => 1, 'name' => 'example.com'));

$coll->update(array('_id' => 1), array('!set' => array('name' => 'google.com')));
$result = $coll->findOne();
echo $result['name'] . "\n";

ini_set('mongo.cmd', '#');
$coll->update(array('_id' => 1), array('#set' => array('name' => 'yahoo.com')));
$result = $coll->findOne();
echo $result['name'] . "\n";
?>
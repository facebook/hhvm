<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'bar');
$coll->drop();

ini_set('mongo.cmd', ':');

$coll->insert(array('_id' => 123, 'hello' => 'world'));
$coll->insert(array('_id' => 456, 'ref' => array(':ref' => 'bar', ':id' => 123)));

$referrer = $coll->findOne(array('_id' => 456));
$referee = MongoDBRef::get($coll->db, $referrer['ref']);
echo $referee['hello'] . "\n";
?>
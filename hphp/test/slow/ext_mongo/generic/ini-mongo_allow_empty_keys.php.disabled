<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'allow_empty_keys');
$coll->drop();

ini_set('mongo.allow_empty_keys', true);

$coll->insert(array('_id' => 1, '' => 'foo'));
$result = $coll->findOne(array('_id' => 1));
var_dump($result['']);
?>
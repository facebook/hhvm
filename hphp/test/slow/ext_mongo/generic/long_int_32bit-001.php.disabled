<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'int');
$coll->drop();

ini_set('mongo.native_long', false);
ini_set('mongo.long_as_object', false);
$coll->insert(array('x' => 9223372036854775807));
$result = $coll->findOne();
var_dump($result['x']);

$coll->drop();

ini_set('mongo.native_long', true);
ini_set('mongo.long_as_object', false);
$coll->insert(array('x' => 9223372036854775807));
$result = $coll->findOne();
var_dump($result['x']);

$coll->drop();

ini_set('mongo.native_long', false);
ini_set('mongo.long_as_object', true);
$coll->insert(array('x' => 9223372036854775807));
$result = $coll->findOne();
var_dump($result['x']);

$coll->drop();

ini_set('mongo.native_long', true);
ini_set('mongo.long_as_object', true);
$coll->insert(array('x' => 9223372036854775807));
$result = $coll->findOne();
var_dump($result['x']);
?>
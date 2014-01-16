<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'mongo_native_long');
$coll->drop();

ini_set('mongo.native_long', true);

$coll->insert(array('int32' => new MongoInt32(1234567890)));
$result = $coll->findOne();
var_dump($result['int32']);

$coll->drop();

ini_set('mongo.native_long', true);

$coll->insert(array('int32' => new MongoInt32(123456789012345)));
$result = $coll->findOne();
var_dump($result['int32']);
?>
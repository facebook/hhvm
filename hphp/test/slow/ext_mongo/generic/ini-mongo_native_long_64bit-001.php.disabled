<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'mongo_native_long');
$coll->drop();

ini_set('mongo.native_long', true);

$coll->insert(array('int64' => 9223372036854775807));
$result = $coll->findOne();
var_dump($result['int64']);
?>
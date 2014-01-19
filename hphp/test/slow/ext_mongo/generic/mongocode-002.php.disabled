<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'mongocode');
$coll->drop();

$coll->insert(array('_id' => 1, 'code' => new MongoCode(3)));
$result = $coll->findOne(array('_id' => 1));
var_dump($result['code']->code);

$coll->insert(array('_id' => 2, 'code' => new MongoCode(null)));
$result = $coll->findOne(array('_id' => 2));
var_dump($result['code']->code);
?>
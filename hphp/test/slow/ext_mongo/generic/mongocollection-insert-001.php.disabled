<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'insert');
$coll->drop();

$coll->insert(array('foo', 'bar'));
$result = $coll->findOne();
var_dump($result['0']);
var_dump($result['1']);

$coll->drop();

$coll->insert(array(-2147483647 => 'xyz'));
$result = $coll->findOne();
var_dump($result['-2147483647']);
?>
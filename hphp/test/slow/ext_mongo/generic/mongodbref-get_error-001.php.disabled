<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$db = $mongo->selectDB('test');

var_dump(MongoDBRef::get($db, null));
var_dump(MongoDBRef::get($db, array()));
var_dump(MongoDBRef::get($db, array('$ref' => 'dbref')));
var_dump(MongoDBRef::get($db, array('$id' => 123)));
?>
<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'dbref');

var_dump($coll->getDBRef(array()));
var_dump($coll->getDBRef(array('$ref' => 'dbref')));
var_dump($coll->getDBRef(array('$id' => 123)));
?>
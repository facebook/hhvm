<?php
require_once __DIR__."/../utils/server.inc";

$m = mongo_standalone();
$collection = $m->selectDb(dbname())->bug635;
var_dump($collection);
 
$cursor = new MongoCursor($m, $collection);
var_dump($collection);
var_dump(is_object($collection));
?>
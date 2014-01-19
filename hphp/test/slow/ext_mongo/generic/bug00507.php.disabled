<?php
require_once __DIR__."/../utils/server.inc";

$dataSize = 3*256*1024;

$data = str_repeat("x", $dataSize);


$m = mongo_standalone();
$db = $m->selectDB(dbname());

$db->selectCollection('PHP507.files')->drop();
$db->selectCollection('PHP507.chunks')->drop();


$grid = $db->getGridFs('PHP507');

$id = $grid->storeBytes($data, array('safe' => true));

var_dump($db->selectCollection('PHP507.chunks')->count(array('files_id' => $id)) > 1);
var_dump($db->selectCollection('PHP507.files')->count(array('_id' => $id)));

$grid->remove(array('_id' => $id), array('justOne' => true));

var_dump($db->selectCollection('PHP507.chunks')->count(array('files_id' => $id)));
var_dump($db->selectCollection('PHP507.files')->count(array('_id' => $id)));
?>
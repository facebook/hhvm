<?php
require_once __DIR__."/../utils/server.inc";

$documents = array(
    array('_id' => 1),
    array('_id' => 2),
);

$m = mongo_standalone();
$c = $m->selectCollection(dbname(), 'bug00776');
$c->drop();

$c->batchInsert($documents, array());

var_dump($c->count());
?>
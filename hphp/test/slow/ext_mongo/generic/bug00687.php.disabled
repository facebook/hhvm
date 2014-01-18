<?php
require_once __DIR__."/../utils/server.inc";

$m = new_mongo_standalone();
$c = $m->selectCollection(dbname(), 'bug687');
$c->drop();

$date = new MongoDate();
$c->insert(array('d' => $date));
$doc = $c->findOne();

var_dump($date == $doc['d']);

?>
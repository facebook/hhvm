<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'mongodate');
$coll->drop();

$date = new MongoDate();

$coll->insert(array('_id' => 1, 'date' => $date));
$result = $coll->findOne(array('_id' => 1));
echo get_class($result['date']) . "\n";
var_dump($result['date']->sec === $date->sec);
var_dump($result['date']->usec === $date->usec);
?>
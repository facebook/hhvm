<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'mongoregex');
$coll->drop();

$regex = new MongoRegex('/foo[bar]{3}/imx');

$coll->insert(array('_id' => 1, 'regex' => $regex));
$result = $coll->findOne(array('_id' => 1));
echo get_class($result['regex']) . "\n";
var_dump($result['regex']->regex === $regex->regex);
var_dump($result['regex']->flags === $regex->flags);
?>
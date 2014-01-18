<?php
require_once __DIR__."/../utils/server.inc";
// Connect
$m = mongo_standalone();
// Select the DB
$db = $m->phpunit;
// Select a collection 
$collection = $db->test;

$collection->drop();

// Add a record 
$obj = array("title" => "test1");

$time = time();
$collection->insert($obj);

$cursor = $collection->find();

$a = $cursor->getNext();

var_dump($time === $a['_id']->getTimestamp());
?>
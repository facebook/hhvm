<?php
require_once __DIR__."/../utils/server.inc";

$m = mongo_standalone();

$db = $m->selectDB(dbname());
$coll = $db->test_sort;
$coll->drop();
// insert test data
for ($i=0; $i<10; $i++) {
    $coll->insert(array('0' => 'a' . $i, '1' => 'b' . $i, '2' => 'c' . $i), array('safe' => true));
}

// ask user on which 'column' to sort
$column = "0";
$sort = -1;

// query and sort
$cursor = $coll->find()->sort(array($column => $sort));
$record = $cursor->getNext();
var_dump($record[0]);

// the line below is a workaround for 1.2.10 or earlier, it casts the array to an object.
$cursor = $coll->find()->sort((object) array($column => $sort));
$record = $cursor->getNext();
var_dump($record[0]);

?>
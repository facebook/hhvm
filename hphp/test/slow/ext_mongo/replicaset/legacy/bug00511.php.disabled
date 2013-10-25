<?php
$mentions = array(); 
require_once __DIR__."/../../utils/server.inc";

$m = mongo();
$db = $m->selectDB(dbname());

MongoLog::setModule( MongoLog::RS );
MongoLog::setLevel( MongoLog::FINE );
MongoLog::setCallback( function($a, $b, $message) { if (preg_match('/connection: type: ([A-Z]+),/', $message, $m )) { @$GLOBALS['mentions'][$m[1]]++; }; } );
$db->setSlaveOkay(true);

$mentions = array();

// Normal find
$ret = $db->safe->find(array("doc" => 1));
iterator_to_array($ret);
var_dump($mentions); $mentions = array();
 
// Force primary for command
$db->setProfilingLevel(42);
var_dump($mentions); $mentions = array();
 
// Normal find
$ret = $db->safe->find(array("doc" => 1));
iterator_to_array($ret);
var_dump($mentions); $mentions = array();
?>
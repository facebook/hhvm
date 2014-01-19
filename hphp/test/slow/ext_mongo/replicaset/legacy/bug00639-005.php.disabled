<?php
$mentions = array(); 
require_once __DIR__."/../../utils/server.inc";

// Has to be old_mongo here, as MongoClient doesn't have the deprecated setSlaveOkay() method.
$m = old_mongo();

MongoLog::setModule( MongoLog::ALL );
MongoLog::setLevel( MongoLog::ALL );

$showNext = false;

MongoLog::setCallback( function($a, $b, $message) use (&$showNext) {
	if ($showNext) {
		echo $message, "\n";
	}
	$showNext = false;
	if (preg_match('/^pick server/', $message)) {
		$showNext = true;
		echo $message, "\n";
	}
} );

$m->setSlaveOkay(true);
$db = $m->selectDB(dbname());
$col = $db->bug639;

$cursor = $col->find(array(), array('email' => true));
$cursor->limit(1);
iterator_to_array($cursor);
$info = $cursor->info();
echo "connection type: ", $info['connection_type_desc'], "\n";

$m->setSlaveOkay(false);
$db = $m->selectDB(dbname());
$col = $db->bug639;

$cursor = $col->find(array(), array('email' => true));
$cursor->limit(1);
iterator_to_array($cursor);
$info = $cursor->info();
echo "connection type: ", $info['connection_type_desc'], "\n";
?>
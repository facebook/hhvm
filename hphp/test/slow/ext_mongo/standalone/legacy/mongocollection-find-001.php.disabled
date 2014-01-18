<?php
function error_handler($code, $message)
{
	echo $message, "\n";
}

set_error_handler('error_handler');

MongoLog::setLevel(MongoLog::ALL);
MongoLog::setModule(MongoLog::ALL);

require_once __DIR__."/../../utils/server.inc";

$mongo = mongo_standalone();
$mongo->safe = true;
$mongo->setReadPreference(Mongo::RP_SECONDARY);

$coll1 = $mongo->selectCollection(dbname(), 'query');
echo "DROPPING:\n";
$coll1->drop();
echo "---\n";
$coll1->insert(array('_id' => 123, 'x' => 'foo'), array('safe' => 1));
echo "---\n";
$coll1->insert(array('_id' => 124, 'x' => 'foo'), array('safe' => 1));
echo "---\n";
$coll1->insert(array('_id' => 125, 'x' => 'foo'), array('safe' => 1));
echo "---\n";
die();

for ($i = 0; $i < 10; $i++) {
	$r = $coll1->find();
	sleep(2);
	echo "--------\n";
}
?>
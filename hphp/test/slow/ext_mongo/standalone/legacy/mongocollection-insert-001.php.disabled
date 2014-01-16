<?php
function error_handler($code, $message)
{
	echo $message, "\n";
}

set_error_handler('error_handler');

MongoLog::setLevel(MongoLog::ALL);
MongoLog::setModule(MongoLog::ALL);

//require_once __DIR__."/../../utils/server.inc";

$mongo = new Mongo("mongodb://%s:%d,%s:%d/?replicaSet=seta");
$mongo->safe = true;
$mongo->setReadPreference(Mongo::RP_NEAREST);

$coll1 = $mongo->selectCollection('phpunit', 'query');
$coll1->drop();

$i = 0;
while ($i < 5) {
	echo "Inserting $i\n";
	try {
		$coll1->insert(array('_id' => $i, 'x' => "foo" . dechex($i)), array('safe' => 1));
	} catch ( Exception $e ) {
		echo get_class( $e ), ': ', $e->getCode(), ', ', $e->getMessage(), "\n";
	}
	$i++;
	sleep(1);
}
?>
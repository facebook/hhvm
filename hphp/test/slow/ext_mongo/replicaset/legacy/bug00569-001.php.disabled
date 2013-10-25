<?php
require_once __DIR__."/../../utils/server.inc";
MongoLog::setModule( MongoLog::IO );
MongoLog::setLevel( MongoLog::FINE );
set_error_handler('foo'); function foo($a, $b) { echo $b, "\n"; };

$m = old_mongo();
$m->selectDB(dbname())->test->remove();

$tests = array(
	array(),
	array( 'safe' => 0 ),
	array( 'safe' => 1 ),
	array( 'safe' => 2 ),
	array( 'safe' => "majority" ),
	array( 'w' => 0 ),
	array( 'w' => 1 ),
	array( 'w' => 2 ),
	array( 'w' => "majority" ),
	array( 'fsync' => 0 ),
	array( 'fsync' => 1 ),
	array( 'fsync' => 0, "w" => 1 ),
	array( 'fsync' => 1, "w" => 0 ),
);

foreach ( $tests as $key => $test )
{
	echo "\nRunning test $key, with options: ", json_encode( $test ), ":\n";
	try
	{
		$m->selectDB(dbname())->test->insert( array( '_id' => $key ), $test );
	}
	catch ( Exception $e )
	{
		echo $e->getMessage(), "\n";
	}
}
?>
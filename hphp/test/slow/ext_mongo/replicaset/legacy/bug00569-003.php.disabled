<?php
require_once __DIR__."/../../utils/server.inc";
MongoLog::setModule( MongoLog::IO | MongoLog::PARSE );
MongoLog::setLevel( MongoLog::FINE | MongoLog::INFO );
set_error_handler('foo'); function foo($a, $b) { echo $b, "\n"; };

$hostname = hostname();
$port = port();

$strings = array(
	"mongodb://$hostname:$port/?w=0",
	"mongodb://$hostname:$port/?w=1",
	"mongodb://$hostname:$port/?w=2",
	"mongodb://$hostname:$port/?w=allDCs",
	"mongodb://$hostname:$port/?w=majority",
);

$tests = array(
	array(),
	array( 'safe' => 0 ),
	array( 'safe' => 1 ),
	array( 'w' => 0 ),
	array( 'w' => 1 ),
);

foreach ( $strings as $string )
{
	echo "\nRunning string $string\n";
	$m = new MongoClient( $string );
	try
	{
		$m->selectDB(dbname())->test->remove();
	}
	catch ( Exception $e )
	{
		echo $e->getMessage(), "\n";
	}
	foreach ( $tests as $key => $test )
	{
		echo "\n- Running test $key, with options: ", json_encode( $test ), ":\n";
		try
		{
			$m->selectDB(dbname())->test->insert( array( '_id' => $key ), $test );
		}
		catch ( Exception $e )
		{
			echo $e->getMessage(), "\n";
		}
	}
}
?>
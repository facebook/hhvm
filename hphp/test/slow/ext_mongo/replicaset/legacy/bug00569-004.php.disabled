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
	0,
	1,
	2,
	7,
	"majority",
	"allDCs",
);

foreach ( $strings as $string )
{
	echo "\nRunning string $string\n";
	$m = new MongoClient( $string );
	$demo = $m->selectDB(dbname());
	try
	{
		$demo->wtimeout = 100;
		$demo->test->remove();
	}
	catch ( Exception $e )
	{
		echo $e->getMessage(), "\n";
	}
	foreach ( $tests as $key => $test )
	{
		echo "\n- Setting w property to $test:\n";
		try
		{
			$demo->w = $test;
			$demo->wtimeout = 100;
			$demo->test->insert( array( '_id' => $key ) );
		}
		catch ( Exception $e )
		{
			echo $e->getMessage(), "\n";
		}
	}
}
?>
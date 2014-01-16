<?php
require_once __DIR__."/../utils/server.inc";
$mongo = mongo_standalone();
$coll = $mongo->selectCollection(dbname(), 'mongodate');
$coll->drop();

$times = array(
	array( 0, 0 ), // epoch
	array( 0, 81234 ),
	array( 0, 801234 ),
	array( 0, 8001234 ),

	array( -1000, 81234 ),
	array( -1000, 801234 ),
	array( -1000, 8001234 ),

	array( -1000, -81234 ),
	array( -1000, -801234 ),
	array( -1000, -8001234 ),

	array( 1000, 81234 ),
	array( 1000, 801234 ),
	array( 1000, 8001234 ),
);

foreach ( $times as $time )
{
	list( $sec, $usec ) = $time;

	echo $sec, ', ', $usec, "\n";

	$a = new MongoDate( $sec, $usec );
	$obj = array( 'date' => $a );

	var_dump( $a );
	echo $a, "\n";
	$coll->insert( $obj );

	$encdec = $coll->findOne();
	var_dump( $encdec['date'] );
	echo $encdec['date'], "\n";
	echo "\n";

	$coll->remove();
}
?>
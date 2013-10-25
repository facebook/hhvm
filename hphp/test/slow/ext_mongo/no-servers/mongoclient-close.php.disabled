<?php
MongoLog::setModule( MongoLog::CON );
MongoLog::setLevel ( MongoLog::INFO );
set_error_handler( function($a, $b, $c) { echo $b, "\n"; } );

function createCons()
{
	global $c, $cons;
	$c = new Mongo('mongodb://localhost:13000', array( 'replicaSet' => 'seta' ));
	$cons = $c->getConnections();
	echo "- available cons:\n";
	array_walk( $cons, function( $a ) { echo $a['hash'], "\n"; } );
}

echo "Testing closing master (BC):\n";
createCons();
echo "closing:\n";
var_dump( $c->close() );
echo "done.\n\n";

echo "Testing closing all (true):\n";
createCons();
echo "closing:\n";
var_dump( $c->close(true) );
echo "done.\n\n";

echo "Testing closing master (false):\n";
createCons();
echo "closing:\n";
var_dump( $c->close(false) );
echo "done.\n\n";

echo "Testing closing per-hash (hash):\n";
createCons();
echo "closing:\n";
foreach ( $cons as $con )
{
	echo "- closing hash {$con['hash']}\n";
	var_dump( $c->close( $con['hash']) );
}
echo "done.\n\n";

echo "Testing closing invalid hash (hash):\n";
createCons();
echo "closing:\n";
var_dump( $c->close( "randomnonsense" ) );
echo "done.\n\n";

echo "Testing closing invalid args):\n";
createCons();
echo "closing:\n";
var_dump( $c->close( array() ) );
var_dump( $c->close( new stdclass() ) );
var_dump( $c->close( fopen('/etc/passwd', 'r') ) );
echo "done.\n\n";
?>
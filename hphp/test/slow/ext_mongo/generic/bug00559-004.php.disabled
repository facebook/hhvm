<?php
MongoLog::setModule( MongoLog::ALL ); MongoLog::setLevel( MongoLog::ALL );
set_error_handler( function( $a, $b ) { echo $b, "\n"; } );
echo "\nCREATING CONNECTION 1\n";
$a = new Mongo("mongodb://localhost:13000/?replicaset=seta");
echo "\nCREATING CONNECTION 2\n";
$b = new Mongo("mongodb://whisky:13000/?replicaset=seta");

echo "\nINSERTING 1\n";
$a->phpunit->test->insert( array( 'foo' => 'bar' ) );
echo "\nINSERTING 2\n";
$b->phpunit->test->insert( array( 'foo' => 'bar' ) );
?>
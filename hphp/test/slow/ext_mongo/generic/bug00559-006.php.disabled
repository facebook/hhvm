<?php
MongoLog::setModule( MongoLog::ALL ); MongoLog::setLevel( MongoLog::ALL );
set_error_handler( function( $a, $b ) { echo $b, "\n"; } );
echo "\n<pre>CREATING CONNECTION\n";
$a = new Mongo("mongodb://whisky:13001/?replicaset=seta&readPreference=nearest");

echo "\nQUERY\n";
$a->phpunit->test->findOne( array( 'foo' => 'bar' ) );

echo "\nINSERT\n";
$a->phpunit->test->insert( array( 'foo' => 'bar' ) );

echo "</pre>\n";
var_dump( $a->getConnections() );

?>
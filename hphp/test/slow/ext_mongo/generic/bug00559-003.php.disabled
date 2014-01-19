<?php
MongoLog::setModule( MongoLog::ALL ); MongoLog::setLevel( MongoLog::ALL );
set_error_handler( function( $a, $b ) { echo $b, "\n"; } );
echo "\nCREATING CONNECTION 1\n";
$a = new Mongo("mongodb://user1:user1@whisky,localhost/phpunit");
echo "\nCREATING CONNECTION 2\n";
$b = new Mongo("mongodb://user2:user2@localhost,whisky/phpunit");
echo "\nCREATING CONNECTION 3\n";
$c = new Mongo("mongodb://user2:user2@whisky,localhost/phpunit");

echo "\nINSERTING 1\n";
$a->phpunit->test->insert( array( 'foo' => 'bar' ) );
echo "\nINSERTING 2\n";
$b->phpunit->test->insert( array( 'foo' => 'bar' ) );
echo "\nINSERTING 3\n";
$c->phpunit->test->insert( array( 'foo' => 'bar' ) );
?>
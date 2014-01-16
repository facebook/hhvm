<?php
require_once __DIR__."/../../utils/server.inc";

// Connect to mongo
$m = mongo_standalone();
$c = $m->selectCollection(dbname(), 'crash');
$c->drop();

var_dump( $c->insert( array( '_id' => 'yeah', 'value' => 'maybe' ) ) );
var_dump( $c->update( array( '_id' => 'yeah' ), array( 'value' => array( '$set' => 'yes!' ) ) ) );
var_dump( $c->remove( array( '_id' => 'yeah' ) ) );

try {
	var_dump( $c->insert( array() ) );
} catch ( Exception $e ) {
	echo "Exception: ", get_class( $e ) , ": ", $e->getMessage(), "\n";
}

var_dump( $c->insert( array( '_id' => 'yeah', 'value' => 'maybe' ) ) );
var_dump( $c->update( array( '_id' => 'yeah' ), array() ) );
var_dump( $c->findOne( array( '_id' => 'yeah' ) ) );

var_dump( $c->remove( array( '_id' => 'yeah' ) ) );
var_dump( $c->findOne( array( '_id' => 'yeah' ) ) );

var_dump( $c->update( array( '_id' => 'yeah' ), array( 'value' => array( '$set' => 'yes!' ) ) ) );
var_dump( $c->findOne( array( '_id' => 'yeah' ) ) );

var_dump( $c->insert( array( '_id' => 'yeah', 'value' => 'maybe' ), array( 'safe' => true ) ) );
var_dump( $c->update( array( '_id' => 'yeah' ), array( 'value' => array( '$set' => 'yes!' ) ), array( 'safe' => true ) ) );
var_dump( $c->remove( array( '_id' => 'yeah' ), array( 'safe' => true ) ) );
?>
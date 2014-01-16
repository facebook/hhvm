<?php
require_once __DIR__."/../utils/server.inc";
$d = mongo_standalone();
$c = $d->phpunit->collection;
$c->drop();

$c->insert( array( '_id' => 'test1', 'value' => 'ONE' ) );
$c->insert( array( '_id' => 'test2', 'value' => 'TWO' ) );
$c->insert( array( '_id' => 'test3', 'value' => 'THREE' ) );

foreach ( $c->find( array(), array( '_id' => 0 ) ) as $key => $value )
{
	var_dump( $key, $value );
}

var_dump( iterator_to_array( $c->find( array(), array( '_id' => 0 ) ) ) );
?>
<?php
require_once __DIR__."/../../utils/server.inc";

$m = mongo_standalone();
$c = $m->selectDb(dbname())->bug605;
$c->remove();
var_dump( $c->insert( array( 'test' => 'one' ) ) );
var_dump( $c->insert( array( 'test' => 'two' ), array( 'safe' => true ) ) );

$m = old_mongo_standalone();
$c = $m->selectDb(dbname())->bug605;
$c->remove();
var_dump( $c->insert( array( 'test' => 'one' ) ) );
var_dump( $c->insert( array( 'test' => 'two' ), array( 'w' => 0 ) ) );
var_dump( $c->insert( array( 'test' => 'two' ), array( 'w' => 1 ) ) );

$m = new_mongo_standalone();
$c = $m->selectDb(dbname())->bug605;
$c->remove();
var_dump( $c->insert( array( 'test' => 'one' ) ) );
var_dump( $c->insert( array( 'test' => 'two' ), array( 'w' => 0 ) ) );
var_dump( $c->insert( array( 'test' => 'two' ), array( 'w' => 1 ) ) );
?>
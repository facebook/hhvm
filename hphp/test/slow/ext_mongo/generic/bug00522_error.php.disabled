<?php
require_once __DIR__."/../utils/server.inc";

$m = mongo_standalone();
$c = $m->selectCollection( dbname(), "php-522_error" );

var_dump( $c->insert( array( 'test' => 1 ), array( 'fsync' => M_PI, 'safe' => M_PI, 'timeout' => "foo" ) ) );
?>
<?php
require_once __DIR__."/../../utils/server.inc";

$m = mongo_standalone();
$c = $m->selectCollection( dbname(), "php-522_error" );

$c->w = "3";

try {
	$c->insert( array( 'test' => 1 ), array( 'safe' => 1 ) );
} catch ( Exception $e ) {
	echo $e->getMessage(), "\n";
}
?>
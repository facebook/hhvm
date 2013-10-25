<?php
require_once __DIR__."/../utils/server.inc";

$m = mongo_standalone(null, true, true, array( 'connect' => false ) );
var_dump($m->connect());

try
{
	$m = new Mongo("mongodb://totallynonsense/", array( 'connect' => false ) );
	var_dump(@$m->connect());
}
catch ( Exception $e )
{
	echo $e->getMessage(), "\n";
}
?>
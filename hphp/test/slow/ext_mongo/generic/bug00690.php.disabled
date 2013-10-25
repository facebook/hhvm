<?php
require_once __DIR__."/../utils/server.inc";
$m = new_mongo_standalone();
$c = $m->selectCollection(dbname(), 'bug690');
$c->drop();

$c->insert(array('_id'=>'hello%20London', 'added'=>time()));
try
{
	$c->insert(array('_id'=>'hello%20London', 'added'=>time()));
}
catch ( Exception $e )
{
	echo $e->getMessage(), "\n";
}

?>
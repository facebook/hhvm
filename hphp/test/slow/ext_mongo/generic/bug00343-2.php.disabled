<?php
require_once __DIR__."/../utils/server.inc";
$m = new_mongo_standalone();
$db = $m->phpunit;
$db->dropCollection( 'phpunit' );
$grid = $db->getGridFS();
$grid->drop();
$saved = $grid->storeFile(
	__FILE__,
	array(
		'filename' => 'test_file-'.rand(0,10000),
		'thumbnail_size' => 'm',
		'otherdata' => 'BIG'
	),
	array('safe' => true)
);
var_dump( $grid->findOne() );
echo "OK\n";
?>
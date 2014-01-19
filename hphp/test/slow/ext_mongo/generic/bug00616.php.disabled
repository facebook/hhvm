<?php
require_once __DIR__."/../utils/server.inc";
    $m = mongo_standalone("phpunit");
	$mdb = $m->selectDB("phpunit");
	$mdb->dropCollection("fs.files");
	$mdb->dropCollection("fs.chunks");

	$GridFS = $mdb->getGridFS();

	$temporary_file_name = tempnam(sys_get_temp_dir(), "gridfs-delete");
	$temporary_file_data = '1234567890';
	file_put_contents($temporary_file_name, $temporary_file_data);

	$ids = array(
		"file0",
		452,
		true,
		new MongoID(),
		array( 'a', 'b' => 5 ),
	);

	foreach ( $ids as $id )
	{
		echo "Using ID:";
		var_dump( $id );
		$GridFS->storeFile($temporary_file_name, array( '_id' => $id));

		echo "Items in DB: ", $GridFS->find()->count(), "\n";
		$GridFS->delete( $id );
		echo "Items in DB: ", $GridFS->find()->count(), "\n";

		echo "\n";
	}
    unlink($temporary_file_name);
?>
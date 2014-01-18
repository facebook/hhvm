<?php
require_once __DIR__."/../utils/server.inc";
$m = mongo_standalone("phpunit");
$mdb = $m->selectDB("phpunit");
$mdb->dropCollection("fs.files");
$mdb->dropCollection("fs.chunks");

$GridFS = $mdb->getGridFS();

$temporary_file_name = '/tmp/GridFS_test.txt';
$temporary_file_data = '1234567890';
file_put_contents($temporary_file_name, $temporary_file_data);

$options = array( 'safe' => false );
for ($i = 0; $i < 3; $i++) {
	try {
		$new_saved_file_object_id = $GridFS->storeFile($temporary_file_name, array( '_id' => "file{$i}", "filename" => "file.txt"), $options);
		echo "[Saved file] New file id:".$new_saved_file_object_id."\n";
	}
	catch (MongoException $e) {
		echo "error message: ".$e->getMessage()."\n";
		echo "error code: ".$e->getCode()."\n";
	}

}

echo "\n";
$cursor = $GridFS->find( array(), array( '_id' => 0 ));
foreach ( $cursor as $key => $item )
{
	echo $key, ': ', $item->file['filename'], "\n";
}
echo "\n";

foreach ( iterator_to_array( $cursor ) as $key => $item )
{
	echo $key, ': ', $item->file['filename'], "\n";
}
?>
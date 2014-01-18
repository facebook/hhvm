<?php
require_once __DIR__."/../utils/server.inc";
$conn = mongo_standalone();
$db   = $conn->selectDb(dbname());
$grid = $db->getGridFs('wrapper');

// delete any previous results
$grid->drop();

// dummy file
$bytes = str_repeat("x", 128);
$grid->storeBytes($bytes, array("filename" => "demo.txt", 'chunkSize' => 128), array('safe' => true));
unset($bytes);

// fetch it
$file = $grid->findOne(array('filename' => 'demo.txt'));
$chunkSize = $file->file['chunkSize'];

$readSizes = array( 8, 16, 31, 32, 33, 127, 128, 129 );

foreach ($readSizes as $size) {
	$fp = $file->getResource();
	while (!feof($fp)) {
		$t = fread($fp, $size);
		if ($size != strlen($t)) {
			echo "read size(", strlen($t), ") is not the same as requested size ($size)\n";
		} else {
			echo "read: ", strlen($t), " bytes\n";
		}
	}
	fclose($fp);
}
?>
<?php

echo "Procedural\n";
$zip = zip_open(dirname(__FILE__) . '/test.zip');
if (!is_resource($zip)) {
	die("Failure");
	}
var_dump(zip_close($zip));
var_dump(zip_close($zip));

echo "Object\n";
$zip = new ZipArchive();
if (!$zip->open(dirname(__FILE__) . '/test.zip')) {
	die('Failure');
}
if ($zip->status == ZIPARCHIVE::ER_OK) {
	var_dump($zip->close());
	var_dump($zip->close());
} else {
	die("Failure");
}

?>
Done

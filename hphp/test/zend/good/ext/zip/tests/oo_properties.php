<?php

$dirname = dirname(__FILE__) . '/';
$file = $dirname . '__property_test.zip';

copy($dirname . 'test_with_comment.zip', $file);

$zip = new ZipArchive;
if (!$zip->open($file)) {
	exit('failed');
}

printf("zip->status (%d):\n\tempty(): %d\n\tisset(): %d\n", $zip->status, empty($zip->status), isset($zip->status));
printf("zip->numFiles (%d):\n\tempty(): %d\n\tisset(): %d\n", $zip->numFiles, empty($zip->numFiles), isset($zip->numFiles));
printf("zip->bogus (%d):\n\tempty(): %d\n\tisset(): %d\n", $zip->bogus, empty($zip->bogus), isset($zip->bogus));


$zip->addEmptyDir('emptydir');

printf("zip->status (%d):\n\tempty(): %d\n\tisset(): %d\n", $zip->status, empty($zip->status), isset($zip->status));
printf("zip->numFiles (%d):\n\tempty(): %d\n\tisset(): %d\n", $zip->numFiles, empty($zip->numFiles), isset($zip->numFiles));
printf("zip->filename (%d):\n\tempty(): %d\n\tisset(): %d\n", strlen($zip->filename), empty($zip->filename), isset($zip->filename));
printf("zip->comment (%d):\n\tempty(): %d\n\tisset(): %d\n", strlen($zip->comment), empty($zip->comment), isset($zip->comment));

unset($zip); //close the file before unlinking
@unlink($file);
?>
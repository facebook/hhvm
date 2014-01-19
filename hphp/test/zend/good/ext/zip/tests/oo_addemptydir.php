<?php

$dirname = dirname(__FILE__) . '/';
include $dirname . 'utils.inc';
$file = $dirname . '__tmp_oo_addfile2.zip';

copy($dirname . 'test.zip', $file);

$zip = new ZipArchive;
if (!$zip->open($file)) {
	exit('failed');
}

$zip->addEmptyDir('emptydir');
if ($zip->status == ZIPARCHIVE::ER_OK) {
	dump_entries_name($zip);
	$zip->close();
} else {
	echo "failed\n";
}
@unlink($file);
?>
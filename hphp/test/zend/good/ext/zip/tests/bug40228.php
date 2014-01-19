<?php
$dest = dirname(__FILE__);
$arc_name = $dest . "/bug40228.zip";
$zip = new ZipArchive;
$zip->open($arc_name, ZIPARCHIVE::CREATE);;
$zip->extractTo($dest);
if (is_dir($dest . '/test/empty')) {
	echo "Ok\n";
	rmdir($dest . '/test/empty');
	rmdir($dest . '/test');
} else {
	echo "Failed.\n";
}
echo "Done\n";
?>
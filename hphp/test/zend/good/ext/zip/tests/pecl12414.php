<?php
$filename = 'MYLOGOV2.GFX';
$zipname = dirname(__FILE__) . "/pecl12414.zip";
$za = new ZipArchive();
$res =$za->open($zipname);
if ($res === TRUE) {
	$finfo=$za->statName($filename);
	$file_size=$finfo['size'];

	if($file_size>0) {
		$contents=$za->getFromName($filename);

		echo "ZIP contents size: " . strlen($contents) . "\n";
		if(strlen($contents)!=$file_size) {
			echo "zip_readfile recorded data does not match unpacked size: " . $zipname . " : " . $filename;
		}
	} else {
		$contents=false;
		echo "zip_readfile could not open stream from zero length file " . $zipname . " : " .$filename;
	}

	$za->close();
} else {
	echo "zip_readfile could not read from " . $zipname . " : " . $filename;
}

?>
<?php
	// Note: SWC requires zlib
	$dir = opendir(dirname(__FILE__)) or die('cannot open directory: '.dirname(__FILE__));
	$result = array();
	$files  = array();
	while (($file = readdir($dir)) !== FALSE) {
		if (preg_match('/^test.+pix\./',$file) && $file != "test13pix.swf") {
			$files[] = $file;
		}
	}
	closedir($dir);
	sort($files);
	foreach($files as $file) {
		$result[$file] = getimagesize(dirname(__FILE__)."/$file");
		$result[$file] = image_type_to_mime_type($result[$file][2]);
	}
	var_dump($result);
?>
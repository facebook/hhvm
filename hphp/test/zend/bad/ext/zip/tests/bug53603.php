<?php

class TestStream {
	function url_stat($path, $flags) {
		if (!($flags & STREAM_URL_STAT_QUIET))
			trigger_error("not quiet");
		return array();
	}
}

stream_wrapper_register("teststream", "TestStream");

$dirname = dirname(__FILE__) . '/';
$file = $dirname . 'test_with_comment.zip';
$zip = new ZipArchive;
if ($zip->open($file) !== TRUE) {
	echo "open failed.\n";
	exit('failed');
}

$a = $zip->extractTo('teststream://test');
var_dump($a);

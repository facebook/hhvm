<?php
	$cwd = dirname(__FILE__);

	echo "XBM to PNG conversion: ";
	echo imagepng(imagecreatefromxbm($cwd . "/conv_test.xbm"), $cwd . "/test_xbm.png") ? 'ok' : 'failed';
	echo "\n";
	
	@unlink($cwd . "/test_xbm.png");
?>
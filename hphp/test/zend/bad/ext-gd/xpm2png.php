<?php
	$cwd = dirname(__FILE__);

	echo "XPM to PNG conversion: ";
	echo imagepng(imagecreatefromxpm($cwd . "/conv_test.xpm"), $cwd . "/test_xpm.png") ? 'ok' : 'failed';
	echo "\n";

	@unlink($cwd . "/test_xpm.png");
?>
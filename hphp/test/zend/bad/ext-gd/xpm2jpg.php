<?php
	$cwd = dirname(__FILE__);

	echo "XPM to JPEG conversion: ";
	echo imagejpeg(imagecreatefromxpm($cwd . "/conv_test.xpm"), $cwd . "/test_xpm.jpeg") ? 'ok' : 'failed';
	echo "\n";

	@unlink($cwd . "/test_xpm.jpeg");
?>
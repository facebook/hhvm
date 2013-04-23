<?php
	$cwd = dirname(__FILE__);

	echo "GIF to JPEG conversion: ";
	echo imagejpeg(imagecreatefromgif($cwd . "/conv_test.gif"), $cwd . "/test_gif.jpeg") ? 'ok' : 'failed';
	echo "\n";

	@unlink($cwd . "/test_gif.jpeg");
?>
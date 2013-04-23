<?php
	$cwd = dirname(__FILE__);

	echo "GIF to PNG conversion: ";
	echo imagepng(imagecreatefromgif($cwd . "/conv_test.gif"), $cwd . "/test_gif.png") ? 'ok' : 'failed';
	echo "\n";

	@unlink($cwd . "/test_gif.png");
?>
<?php
	$cwd = dirname(__FILE__);

	echo "GIF to GD1 conversion: ";
	echo imagegd(imagecreatefromgif($cwd . "/conv_test.gif"), $cwd . "/test.gd1") ? 'ok' : 'failed';
	echo "\n";

	echo "GIF to GD2 conversion: ";
	echo imagegd2(imagecreatefromgif($cwd . "/conv_test.gif"), $cwd . "/test.gd2") ? 'ok' : 'failed';
	echo "\n";

	@unlink($cwd . "/test.gd1");
	@unlink($cwd . "/test.gd2");
?>
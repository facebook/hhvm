<?php
	$cwd = dirname(__FILE__);

	echo "XPM to GD1 conversion: ";
	echo imagegd(imagecreatefromxpm($cwd . "/conv_test.xpm"), $cwd . "/test.gd1") ? 'ok' : 'failed';
	echo "\n";

	echo "XPM to GD2 conversion: ";
	echo imagegd2(imagecreatefromxpm($cwd . "/conv_test.xpm"), $cwd . "/test.gd2") ? 'ok' : 'failed';
	echo "\n";

	@unlink($cwd . "/test.gd1");
	@unlink($cwd . "/test.gd2");
?>
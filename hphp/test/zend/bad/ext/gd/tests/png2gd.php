<?php
	$cwd = dirname(__FILE__);

	echo "PNG to GD1 conversion: ";
	echo imagegd(imagecreatefrompng($cwd . "/conv_test.png"), $cwd . "/test.gd1") ? 'ok' : 'failed';
	echo "\n";

	echo "PNG to GD2 conversion: ";
	echo imagegd2(imagecreatefrompng($cwd . "/conv_test.png"), $cwd . "/test.gd2") ? 'ok' : 'failed';
	echo "\n";

	echo "GD1 to PNG conversion: ";
	echo imagepng(imagecreatefromgd($cwd . "/test.gd1"), $cwd . "/test_gd1.png") ? 'ok' : 'failed';
	echo "\n";

	echo "GD2 to PNG conversion: ";
	echo imagepng(imagecreatefromgd2($cwd . "/test.gd2"), $cwd . "/test_gd2.png") ? 'ok' : 'failed';
	echo "\n";

	@unlink($cwd . "/test.gd1");
	@unlink($cwd . "/test.gd2");
	@unlink($cwd . "/test_gd1.png");
	@unlink($cwd . "/test_gd2.png");
?>
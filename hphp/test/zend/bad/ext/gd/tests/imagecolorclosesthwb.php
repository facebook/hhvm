<?php
	$im = imagecreatefrompng(dirname(__FILE__).'/test.png');

	var_dump(imagecolorclosesthwb($im, 255, 50, 0));

	var_dump(imagecolorclosesthwb(NULL));
	var_dump(imagecolorclosesthwb(NULL, NULL, NULL, NULL));
	var_dump(imagecolorclosesthwb($im, "hello", "from", "gd"));

	imagedestroy($im);
?>
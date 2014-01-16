<?php
ini_set('open_basedir', /);

	$dir = dirname(__FILE__);
	$a=glob($dir . "/test.*");
	print_r($a);
?>
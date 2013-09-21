<?php
	$a = array(1, 2, 3);
	$r = &$a[0];
	$r = &$a[1];
	$r = &$a[2];
	var_dump($a);
?>
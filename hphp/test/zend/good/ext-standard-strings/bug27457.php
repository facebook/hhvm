<?php
	$test = "Dot in brackets [.]\n";
	echo $test;
	$test = strtr($test, array('.' => '0'));
	echo $test;
	$test = strtr($test, array('0' => '.'));
	echo $test;
	$test = strtr($test, '.', '0');
	echo $test;
	$test = strtr($test, '0', '.');
	echo $test;
?>
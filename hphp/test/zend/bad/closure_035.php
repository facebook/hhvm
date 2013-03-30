<?php

$x = function () use (&$x) {
	$h = function () use ($x) {
		var_dump($x);
		return 1;
	};	
	return $h();
};

var_dump($x());

?>
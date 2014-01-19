<?php

var_dump((array) 1);
var_dump((array) NULL);
var_dump((array) new stdclass);
var_dump($h = (array) function () { return 2; });
var_dump($h[0]());

$i = function () use (&$h) {
	return $h;
};

var_dump($x = (array)$i);
var_dump($y = $x[0]);
var_dump($y());

$items = range(1, 5);
$func = function(){ return 'just a test'; };

array_splice($items, 0 , 4, $func);
var_dump($items);

?>
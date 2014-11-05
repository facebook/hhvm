<?php
$x = array('abc', 'ddd', 'bbb', 10, 'www', 0);
sort($x);
var_dump($x);

$x = array('abc', 'ddd', 'bbb', 0, 'www', 10);
sort($x);
var_dump($x);

 $x = array('abc', 0, 'aaa', 10);
sort($x);
var_dump($x);

$x = array(0, 'aaa', 10, 5, 'a', 'wat');
sort($x);
var_dump($x);

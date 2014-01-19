<?php

$ret = range(0, 12);
var_dump(count($ret));
var_dump($ret[0]);
var_dump($ret[12]);

// The step parameter was introduced in 5.0.0
$ret = range(0, 100, 10);
var_dump(count($ret));
var_dump($ret[0]);
var_dump($ret[5]);
var_dump($ret[10]);

// Use of character sequences introduced in 4.1.0
// array("a", "b", "c", "d", "e", "f", "g", "h", "i");
$ret = range("a", "i");
var_dump(count($ret));
var_dump($ret[0]);
var_dump($ret[4]);
var_dump($ret[8]);

var_dump(range("c", "a"));



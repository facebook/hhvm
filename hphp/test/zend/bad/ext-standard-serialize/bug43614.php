<?php

error_reporting(E_ALL);

var_dump($a = unserialize('a:2:{s:2:"10";i:1;s:2:"01";i:2;}'));
var_dump($a['10']);
var_dump($a[b'01']);

?>
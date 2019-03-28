<?php
$x = array();
$y = array(&$x[0]);
$y[0] = 2;
var_dump($x);

<?php

var_dump(empty($GLOBALS));
$a = array('a' => 'b');
var_dump(empty($a));
$b = array('a');
var_dump(empty($b));
$c = array();
var_dump(empty($c));

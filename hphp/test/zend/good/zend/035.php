<?php

static $var, $var, $var = -1;
var_dump($var);

global $var, $var, $var;
var_dump($var);

var_dump($GLOBALS['var']);

?>
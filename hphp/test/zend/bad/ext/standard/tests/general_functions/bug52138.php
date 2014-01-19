<?php

define('MYCONST', 1);
define('A', 'B');

$ini_file = dirname(__FILE__)."/bug52138.data";

$ret = parse_ini_file($ini_file, true);
var_dump($ret);

?>
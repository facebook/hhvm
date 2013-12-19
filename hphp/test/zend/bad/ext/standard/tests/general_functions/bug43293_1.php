<?php
ini_set('register_argc_argv', Off);

$argv = array(1, 2, 3); 
var_dump(getopt("abcd"));
var_dump($argv);
$argv = null;
var_dump(getopt("abcd"));
?>
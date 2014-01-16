<?php
ini_set('register_argc_argv', Off);

$argv = array(true, false);
var_dump(getopt("abcd"));
?>
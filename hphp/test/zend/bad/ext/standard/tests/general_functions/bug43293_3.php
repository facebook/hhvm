<?php
ini_set('register_argc_argv', On);

$args = array(true, false, "f");
var_dump(getopt("f", $args), $args);
?>
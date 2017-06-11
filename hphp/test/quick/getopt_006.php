<?php
$argv = [$argv[0], '--arg', 'val', 'hello'];
var_dump(getopt("a:", array("arg:"), $optind));
var_dump($optind);
$argv = [$argv[0]];
var_dump(getopt("a:", array("arg:"), $optind));
var_dump($optind);
$argv = [$argv[0], '--invalid'];
var_dump(getopt("a:", array("arg:"), $optind));
var_dump($optind);
$argv = [$argv[0], '-i', '3'];
var_dump(getopt("a:", array("arg:"), $optind));
var_dump($optind);
$argv = [$argv[0], '-a', '3'];
var_dump(getopt("a:", array("arg:"), $optind));
var_dump($optind);
$argv = [$argv[0], '-a', '3', 'extra'];
var_dump(getopt("a:", array("arg:"), $optind));
var_dump($optind);
?>

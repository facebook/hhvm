<?php

const SCRIPT_NAME = __DIR__.'/command_line_arguments.inc';

$cmd = PHP_BINARY.' --php -n -d foo=bar '.SCRIPT_NAME;
var_dump($cmd);
system($cmd);

$cmd = PHP_BINARY.' --php -n < '.SCRIPT_NAME;
var_dump($cmd);
system($cmd);

<?php
// Since we read in the environment variable on request init and we are changing
// default values only, we have to start a new process to test this
$xdebug_config = implode(" ", [
  "profiler_enable=1",
  "profiler_output_dir=/tmp"
]);

$env = "XDEBUG_CONFIG=\"$xdebug_config\"";
$hhvm = PHP_BINARY;
$file = __DIR__ . '/xdebug_config_testee.inc';
$opts = "-c $file.ini";
$cmd = "$env $hhvm $opts $file";
system($cmd);

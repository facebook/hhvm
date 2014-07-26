<?php
// Since xdebug writes profiling results on request shutdown, we have to
// manually run a php process to test profiling output results
$hhvm = PHP_BINARY;
$file = __DIR__ . '/profiling_testee.inc';
$opts = "-c $file.ini";
$cmd = "$hhvm $opts $file";
$out = exec($cmd);
var_dump(file_get_contents($out));

<?php

if ($argc == 1) {
  var_dump($argc, count($argv));
  var_dump($_SERVER['argc'], count($_SERVER['argv']));

  $cmd = file_get_contents('/proc/self/cmdline');
  $cmd = str_replace("\000--file\000", ' ', $cmd);
  $cmd = str_replace("\000", ' ', $cmd);
  $cmd = sprintf('%s argument', $cmd);

  echo shell_exec($cmd);
} else {
  var_dump($argc, count($argv));
  var_dump($_SERVER['argc'], count($_SERVER['argv']));
}
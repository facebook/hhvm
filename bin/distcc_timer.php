<?php

$file = isset($argv[1]) ? $argv[1] : '/tmp/distcc_timer.log';
$lines = array();
exec("cat $file", $lines);
$time = array();
$machines = array();
foreach ($lines as $line) {
  preg_match('/([^ ]+) @ ([^ ]+)/', $line, $m);
  $filename = $m[1];
  $machine = $m[2];
  if (!isset($machines[$machine])) $machines[$machine] = 0;
  ++$machines[$machine];
  preg_match('/pp start: +([0-9]+)/',     $line, $m); $pp0 = $m[1];
  preg_match('/pp end: +([0-9]+)/',       $line, $m); $pp1 = $m[1];
  preg_match('/remote start: +([0-9]+)/', $line, $m); $cc0 = $m[1];
  preg_match('/remote end: +([0-9]+)/',   $line, $m); $cc1 = $m[1];
  $preprocessing = $pp1 - $pp0;
  $compiling = $cc1 - $cc0;
  if ($preprocessing >= 10) {
    echo "preprocessing: $preprocessing, compiling: $compiling  --> $filename\n";
  }
  $time[$filename] = $compiling;
}

function cmp($a, $b) {
  if ($a === $b) return 0;
  return ($a > $b) ? 1 : -1;
}

uasort($time, 'cmp');
foreach ($time as $filename => $compiling) {
  echo "compiling: $compiling  --> $filename\n";
}
var_dump($machines);

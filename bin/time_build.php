<?php

$lines = file_get_contents($argv[1]);
$lines = preg_split('/\n/', $lines);

$times = array();
foreach ($lines as $line) {
  if (preg_match('/^([0-9\.]+) .* ([^ ]+\.cpp|c)$/', $line, $matches)) {
    $time = $matches[1];
    $file = $matches[2];
    $times[$file] = $time;
  } else if (preg_match('/^([0-9\.]+) (g\+\+ -o|ar -crs)/', $line, $matches)) {
    $linktime = $matches[1];
  } else {
    print "Unknown output: $line";
  }
}

asort($times);
foreach ($times as $file => $time) {
  print format_time($time)." compiling $file\n";
}
print format_time($linktime)." linking\n";

function format_time($time) {
  return (int)($time / 60) . "'" .
    (($time % 60) > 9 ? '':'0'). ($time % 60) . '"';
}

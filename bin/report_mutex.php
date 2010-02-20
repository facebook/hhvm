<?php

$server = $argv[1];
$top = $argv[2];
$translate = $argv[3];
if (!$top) $top = 20;

$ret = shell_exec("GET 'http://$server/stats.kvp?agg=*&keys=:mutex.*:'");
$stats = json_decode($ret);
if (!$stats) {
  exit("No mutex profile data was found on server\n");
}

foreach ($stats as $name => $count) {
  if (preg_match('/mutex.([0-9a-f:]+).(hit|time)/', $name, $m)) {
    $stack = $m[1];
    $type = $m[2];

    if ($type == 'hit') {
      $hits[$stack] = $count;
    } else {
      $times[$stack] = $count;
    }
  }
}

arsort($hits); $hits = array_slice($hits, 0, $top);
arsort($times); $times = array_slice($times, 0, $top);

$thits = array();
print str_repeat('=', 70)."\n";
foreach ($hits as $stack => $count) {
  print $count ." x sampling hits:\n";
  print $translate ? translate_stack($stack) : $stack."\n";
  print str_repeat('-', 70)."\n";
}
$ttimes = array();
print str_repeat('=', 70)."\n";
foreach ($times as $stack => $count) {
  print (int)($count/1000000) ." seconds:\n";
  print $translate ? translate_stack($stack) : $stack."\n";
  print str_repeat('-', 70)."\n";
}

function translate_stack($stack) {
  global $server;
  return shell_exec("GET http://$server/translate?stack=$stack");
}


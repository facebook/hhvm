<?php

$count = 0;

function trace($x) {
  global $count;
  echo $count . " " . $x . "\n";
  $count ++;
  return $x;
}

$x = array("foo" => "bar");

$y = $x["foo"];
$x[trace("bar")] = trace("quux");
$x["true"] = 4;
$x[] = 10;
$x[2] = 35;

var_dump($x);
var_dump($x["bar"]);
var_dump($y);

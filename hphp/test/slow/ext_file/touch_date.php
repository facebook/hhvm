<?php

function inRange($x, $a, $b) {
  return ($x >= $a && $x <= $b) ? "YES" : "NO";
}

// No args
$now = time();
touch("/tmp/test0");
$fileInfo = new SplFileInfo("/tmp/test0");
print(inRange($fileInfo->getMTime(), $now, $now + 10)."\n");
print(inRange($fileInfo->getATime(), $now, $now + 10)."\n");


// Mofification time only
touch("/tmp/test1", strtotime("@100200300"));
$fileInfo = new SplFileInfo("/tmp/test1");
print($fileInfo->getMTime()."\n");
print($fileInfo->getATime()."\n");


// Modification and access time
touch("/tmp/test2", strtotime("@100200300"), strtotime("@100400500"));
$fileInfo = new SplFileInfo("/tmp/test2");
print($fileInfo->getMTime()."\n");
print($fileInfo->getATime()."\n");

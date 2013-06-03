<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

function foo($x){
  $a = 1;
  $b = 2;
  $c = 3;
  return $$x;
}

$s = foo("a");
echo $s;
$s = foo("b");
echo $s;
$s = foo("c");
echo $s;
echo "\n";
$s = foo("d");
echo $s;


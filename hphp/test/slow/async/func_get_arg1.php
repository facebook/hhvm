<?php

async function g(){
  return func_num_args();
}

async function f() {
  $a = await g(1,2,3);
  var_dump($a);
  $b = await g();
  var_dump($b);
  return func_get_arg(2);
}

$wh = f("a","b","c");
$r = $wh->join();
var_dump($r);

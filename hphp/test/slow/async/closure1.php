<?php

async function bar() {
  return 1;
}

$foo = async function() {
  $b = await bar();
  return 1 + $b;
};

$wh = $foo();
$r = $wh->join();
var_dump($r);

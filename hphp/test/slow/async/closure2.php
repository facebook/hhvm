<?php

async function foo() {
  $bar = async function($a) {
    return $a;
  };
  $b = await $bar(13);
  return 1 + $b;
}

$wh = foo();
$res = $wh->join();
var_dump($res);

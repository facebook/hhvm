<?php

async function bar() {
  return 1;
}

async function foo() {
  $b = await bar();
  return 1 + $b;
}

$barwh = bar();
$barres = $barwh->join();
var_dump($barres);

$foowh = foo();
$foores = $foowh->join();
var_dump($foores);

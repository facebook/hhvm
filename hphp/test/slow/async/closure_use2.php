<?php

$env = 3;

$g = async function () use ($env) {
  return ++$env;
};

$f = async function ($a) use ($env, $g) {
  var_dump($a);
  var_dump($a + ($env++));
  $b = await $g();
  return $b + ($env++);
};

$r = $f(100)->join();
var_dump($r);

$r = $f(100)->join();
var_dump($r);

var_dump($env);

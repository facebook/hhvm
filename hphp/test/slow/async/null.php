<?php

async function bar() {
  await null;
  return 1;
}

async function bas() {
  await null;
}

async function foo() {
  $a = await null;
  var_dump($a);
  $a = await bar();
  var_dump($a);
  $n = await bas();
  $a = await $n;
  var_dump($a);
}

var_dump(foo()->join());


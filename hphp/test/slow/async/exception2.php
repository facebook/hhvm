<?php

async function eee() {
  throw new Exception("bah!");
}

async function f() {
  $a = eee();
  await $a;
}

try {
  $a = f();
  $b = $a->join();
}
catch (Exception $e) {
  var_dump($a->getexceptioniffailed()->getMessage());
}


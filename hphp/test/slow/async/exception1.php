<?php

async function eee() {
  throw new Exception("bah!");
}

async function f() {
  $a = eee();
  try {
    await $a;
    return "nothing";
  }
  catch (Exception $e) {
    return $e->getMessage();
  }
}

var_dump(f()->join());

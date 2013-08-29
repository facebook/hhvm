<?php

class F {
  async function foo() {
    $bar = async function () {
      return 2;
    };
    $b = await $bar();
    return 1 + $b;
  }
}

$f = new F;
$v = $f->foo()->join();
var_dump($v);

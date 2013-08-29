<?php

class F {
  async function bar() {
    return 1;
  }

  async function foo() {
    $b = await F::bar();
    return 1 + $b;
  }
}

$barwh = F::bar();
$barres = $barwh->join();
var_dump($barres);

$foowh = F::foo();
$foores = $foowh->join();
var_dump($foores);

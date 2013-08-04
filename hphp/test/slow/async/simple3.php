<?php

class F {
  function __construct($a) {
    $this->a = $a;
  }

  async function bar() {
    return $this->a;
  }

  async function foo() {
    $b = await $this->bar();
    return 1 + $b;
  }
}

$f = new F(42);

$barwh = $f->bar();
$barres = $barwh->join();
var_dump($barres);

$foowh = $f->foo();
$foores = $foowh->join();
var_dump($foores);

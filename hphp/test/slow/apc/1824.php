<?php

class A {
  public function gen($a, $b) {
    yield $a;
    yield $b;
  }
}

$x = new A;
$x->cache_gen = $x->gen('a', 'b');
foreach ($x->cache_gen as $v) {
 var_dump($v);
 }
apc_store('key', $x);
$y = apc_fetch('key');
var_dump($y->cache_gen);

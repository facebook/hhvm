<?php

class X {
  function foo($t) {
    $$t = 5;
    yield $this;
  }
}
$x = new X;
foreach ($x->foo('this') as $v) {
 var_dump($v);
 }

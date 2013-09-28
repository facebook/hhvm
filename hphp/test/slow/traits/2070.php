<?php

trait T {
  function foo($t) {
    $$t = 5;
    yield $this;
  }
}
class X {
 use T;
 }
$x = new X;
foreach ($x->foo('this') as $v) {
 var_dump($v);
 }

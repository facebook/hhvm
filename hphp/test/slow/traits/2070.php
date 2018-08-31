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

<<__EntryPoint>>
function main_2070() {
$x = new X;
foreach ($x->foo('this') as $v) {
 var_dump($v);
 }
}

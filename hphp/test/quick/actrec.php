<?php

class X {
  private $bar=1;
  function __destruct() {
    var_dump(__METHOD__);
    global $e;
    $e = debug_backtrace(true);
  }
  function foo($ids) {
    return array($this->bar,
                 $ids,
                 $this->bar,
                 $this->bar,
                 $this->bar,
                 $this->bar);
  }
}

function test() {
  $a = new X;
  yield 1;
  yield $a;
  global $g;
  $g = null;
  yield 2;
}

function main() {
  global $g;
  $g = test();
  for ($g->next(); $g && $g->valid(); $g->next())
    var_dump($g->current());
  var_dump($g);
  global $e;
  $e = null;
}

main();
$a = new X;
var_dump($a->foo(1));
$a = null;

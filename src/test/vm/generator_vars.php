<?php

class logger {
  static $x = 0;
  private $idx;
  function __construct() {
    $this->idx = self::$x++;
    printf("logger %d constructing\n", $this->idx);
  }
  function __destruct() {
    printf("logger %d destructing\n", $this->idx);
  }
}

function create() {
  $x = 5;
  yield $x;
  $s = 'foo';
  yield $s;
  $$s = 1234;
  $z = $$s;
  yield $z;
  yield $$s;
}

function unusedarg($x, $y) {
  $z = 5;
  yield compact('x', 'z');
  $s = 'foo';
  yield 'almost there';
  $$s = 'inside foo';
  yield compact('foo', 's');
  yield compact('x', 'y', 'foo', 'z');
}

function dumpgen($g) {
  foreach ($g as $v) {
    var_dump($v);
  }
}

function getargs($foo) {
  yield 0xdeadbeef;
  yield func_get_args();
  yield func_get_arg(3);
}

function genthrow() {
  throw new Exception();
  yield 5;
}

function main() {
  dumpgen(create());
  dumpgen(unusedarg(new logger(), 5));
  dumpgen(getargs(1, 2, 3, 4, 5));
  $g = genthrow();
  try {
    $g->next();
  } catch (Exception $e) {}
  try {
    $g->next();
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
main();

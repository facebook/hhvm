<?php

function dumpEach($gen) {
  foreach ($gen as $v) {
    var_dump($v);
  }
}

function dumpClosure($f) {
  echo $f(), $f(), $f(), "\n";
}

function makeClosureGen() {
  return function () {
    static $x = 0;
    static $y = 0;
    yield $x++;
    yield $y++;
    yield $x++;
    yield $y++;
  };
}

echo "\ngenerator closure\n";
$cg = makeClosureGen();
$cgg = $cg();
dumpEach($cgg);
$cgg = null;
$cgg = $cg();
dumpEach($cgg);
$cg = makeClosureGen();
$cgg = $cg();
dumpEach($cgg);
$cgg = null;
$cgg = $cg();
dumpEach($cgg);

function makeClosure() {
  return function() {
    static $x = 0;
    return $x++;
  };
}

echo "\nplain closure:\n";
$c = makeClosure();
dumpClosure($c);
$c = null;
$c = makeClosure();
dumpClosure($c);

function makeGen() {
  static $x = 0;
  yield $x++;
  yield $x++;
}

echo "\nplain generator:\n";
$g = makeGen();
dumpEach($g);
$g = makeGen();
dumpEach($g);

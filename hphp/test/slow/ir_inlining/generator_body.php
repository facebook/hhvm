<?php

class CGetM {
  private $x;

  public function __construct() {
    $this->x = "asdasd";
  }

  function getX() {
    return $this->x;
  }

  public function genVarious() {
    $local = $this->getX();
    yield "a";
    yield $local;
    yield "c";
  }

  public function genInts($x) {
    yield $x;
    yield $x + 1;
  }
}

function foo() {
  $k = new CGetM();
  $z = $k->getX();
  yield $z;
  yield "\n";
}

function main() {
  foreach (foo() as $x) {
    echo $x;
  }

  $blah = new CGetM;
  foreach ($blah->genVarious() as $x) {
    echo $x;
  }

  $blah = new CGetM;
  foreach ($blah->genInts(666) as $y) {
    echo $y;
  }
}

main();
echo "\n";

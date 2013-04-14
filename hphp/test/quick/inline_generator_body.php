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
}

main();
echo "\n";

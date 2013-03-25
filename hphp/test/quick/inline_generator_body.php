<?php

class CGetM {
  private $x;

  public function __construct() {
    $this->x = "asdasd";
  }

  function getX() {
    return $this->x;
  }
}

function foo() {
  $k = new CGetM();
  $z = $k->getX();
  yield $z;
  yield "\n";
}

foreach (foo() as $x) {
  echo $x;
}

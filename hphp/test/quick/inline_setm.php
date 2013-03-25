<?php

class SetM {
  private $x;

  public function __construct() {
    $this->x = "asdasd";
  }

  public function clearX() {
    $this->x = null;
  }
}

function main() {
  $x = new SetM();
  $x->clearX();
}

main();

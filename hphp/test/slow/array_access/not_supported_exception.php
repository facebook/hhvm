<?php

class Lal {
}

class Foo {
  private $x = null;

  public function __construct() {
    $this->x = new Lal;
  }

  public function sup() {
    $this->x[] = 2;
  }
}

function main() {
  $x = new Foo;
  $x->sup();
}

main();

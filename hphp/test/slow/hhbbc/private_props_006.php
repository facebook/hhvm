<?php

/*
 * Testing 86pinit's effects on private property type inference.
 */

if (mt_rand()) {
  class Bar {
    const A = "asd";
  }
} else {
  class Bar {
    const B = 42;
  }
}

class Testing86Pinit {
  private $x = Bar::A;

  public function asd() {
    if ($this->x) {
      echo "this should happen\n";
    } else {
      echo "hmm\n";
    }
  }
}

function main() {
  $x = new Testing86Pinit();
  $x->asd();
}

main();

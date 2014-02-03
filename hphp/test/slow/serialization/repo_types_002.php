<?php

class A {}
class B {}

class Foob {
  private $x;
  public function __construct() {
    $this->x = new A;
  }
}

function main(Foob $y) {
  echo "heh\n";
}

$l = "O:4:\"Foob\":1:{s:7:\"\000Foob\000x\";O:1:\"A\":0:{}}";
$y = unserialize($l);
var_dump($y);

main($y);

<?php

class Foob {
  private $x = false;
  public function __construct() {}
}

function main(Foob $y) {
  echo "heh\n";
}

$l = "O:4:\"Foob\":1:{s:7:\"\000Foob\000x\";s:5:\"heheh\";}";
$y = unserialize($l);
var_dump($y);

main($y);

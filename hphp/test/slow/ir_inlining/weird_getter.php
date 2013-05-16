<?php

class WeirdGetter {
  private $x;

  public function __construct() {
    $this->x = "some string";
  }

  public function getX($ignored, $arg, $list, $lol) {
    return $this->x;
  }
}

function test8() {
  $k = new WeirdGetter();
  echo "blah" . $k->getX("string", "a", "string", "a");
  echo "\n";
}

test8();

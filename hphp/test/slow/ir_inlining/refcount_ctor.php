<?php

class Obj {
  public function __construct() {
    $this->uniqueVar = "a string";
  }
  private $uniqueVar;
}

function test() {
  new Obj();
  echo "done\n";
}

test();

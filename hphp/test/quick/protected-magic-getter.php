<?php

class Whatever {
  protected $blah;

  public function __get($name) { var_dump($name); }
}

function main() {
  $l = new Whatever();
  $l->blah += 2;
  var_dump($l);
}

main();

<?php

class Whatever {
  public function & __get($name) {
    var_dump($name); global $lol; return $lol;
  }
}

$lol = 12;

function main() {
  $l = new Whatever();
  $l->blah += 1;
  var_dump($l);
}

main();
var_dump($lol);

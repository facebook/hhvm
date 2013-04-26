<?php

class IssetM {
  public function hasX() {
    return isset($this->x);
  }
}

function main() {
  $k = new IssetM();
  $v = $k->hasX();
  var_dump($v);
}

main();

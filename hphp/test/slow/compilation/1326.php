<?php

class C {
  public function loadAllWithIDs($ids) {
    if (!count($ids = array_filter($ids))) {
      return array();
    }
    var_dump('muy malo', $ids);
    return -666;
  }
}

function main() {
  $testA = array(4 => false, 5 => false);
  $c = new C();
  var_dump($c->loadAllWithIDs($testA));
}

main();

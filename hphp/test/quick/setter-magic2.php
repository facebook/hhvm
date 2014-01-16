<?php

class Heh {
  public function __set($k, $v) {
    var_dump($k, $v);
    test();
  }
}

function test() {
  global $heh;
  $heh->prop = 3;
}

$heh = new Heh;
$heh->prop = 2;
var_dump($heh);

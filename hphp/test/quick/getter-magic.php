<?php

error_reporting(-1);

class Heh {
  protected $prop;

  public function __get($k) {
    var_dump($k);
    test();
  }
}

function test() {
  global $heh;
  var_dump($heh->prop);
}

$heh = new Heh;
var_dump($heh->prop);
var_dump($heh);

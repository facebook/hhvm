<?php

error_reporting(-1);

class Heh {
  protected $prop;

  public function __set($k, $v) {
    var_dump($k, $v);
    test();
  }
}

function test() {
  global $heh;
  // Note: this is different from zend right now.  Zend drops this set
  // without a notice or fatal.  We are raising a fatal since we can't
  // access the protected property.
  $heh->prop = 3;
}

$heh = new Heh;
$heh->prop = 2;
var_dump($heh);

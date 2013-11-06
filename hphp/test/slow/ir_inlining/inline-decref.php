<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class d {
  public function __construct() {
    echo "d constructing\n";
  }

  public function __destruct() {
    $f0 = 0;
    $f1 = 1;
    $f2 = 2;
    echo "d destructing\n";
  }

  public $prop;
}

function fun(d $o) {
  $o->prop = null;
}

function main($c1, $c2, $c3, $c4) {
  $o = new d;
  $o->prop = new d;
  fun($o);
  var_dump($c1, $c2, $c3, $c4);
}

main('ceeone', 'ceetwo', 'ceethree', 'ceefour');

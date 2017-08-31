<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class Dtor {
  public $id;
  function __construct($id) {
    $this->id = $id;
  }
  function __destruct() {
    echo "Dtor::__destruct(" . $this->id . ")\n";
  }
}

function main($a, $b, $c) {
  $v = vec[new Dtor($a), new Dtor($b), new Dtor($c)];
  var_dump($v);
}
main(1, 2, 3);

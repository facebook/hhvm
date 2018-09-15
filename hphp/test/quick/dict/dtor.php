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
  $d = dict[1 => new Dtor($a), 2 => new Dtor($b), 3 => new Dtor($c)];
  var_dump($d);
}
main(1, 2, 3);

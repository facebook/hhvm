<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Cls {
  public $idx;
  function __construct($idx) {
    $this->idx = $idx;
  }
  function __destruct() {
    echo "Cls::__destruct " . $this->idx . "\n";
  }
}

function from_obj($obj) {
  return vec($obj);
}

function test() {
  for ($i = 0; $i < 10; $i++) {
    echo "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n";
    from_obj(new Cls($i));
    echo "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
  }
}

error_reporting(0);
test();

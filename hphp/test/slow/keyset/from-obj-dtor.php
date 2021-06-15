<?hh // decl
// Copyright 2004-present Facebook. All Rights Reserved.

class Cls implements Iterator {
  public $idx;
  function __construct($idx) {
    $this->idx = $idx;
  }

  public function rewind() {}
  public function current() {}
  public function key() {}
  public function next() {}
  public function valid() { return false; }
}

function from_obj($obj) {
  return keyset($obj);
}

function test() {
  for ($i = 0; $i < 10; $i++) {
    echo "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n";
    from_obj(new Cls($i));
    var_dump(HH\objprof_get_data());
    echo "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
  }
}


<<__EntryPoint>>
function main_from_obj_dtor() {
error_reporting(E_ERROR);
test();
}

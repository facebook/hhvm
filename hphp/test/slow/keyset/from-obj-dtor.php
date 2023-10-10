<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Cls implements Iterator {
  public $idx;
  function __construct($idx) {
    $this->idx = $idx;
  }

  public function rewind() :mixed{}
  public function current() :mixed{}
  public function key() :mixed{}
  public function next() :mixed{}
  public function valid() :mixed{ return false; }
}

function from_obj($obj) :mixed{
  return keyset($obj);
}

function test() :mixed{
  for ($i = 0; $i < 10; $i++) {
    echo "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n";
    from_obj(new Cls($i));
    var_dump(HH\objprof_get_data());
    echo "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
  }
}


<<__EntryPoint>>
function main_from_obj_dtor() :mixed{
error_reporting(E_ERROR);
test();
}

<?hh // decl
// Copyright 2004-present Facebook. All Rights Reserved.

class Cls implements Iterator {
  public function rewind() {}
  public function current() {}
  public function key() {}
  public function next() {}
  public function valid() { return false; }
}

function from_obj($obj) {
  return dict($obj);
}

function test() {
  for ($i = 0; $i < 10; $i++) {
    echo "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n";
    from_obj(new Cls);
    var_dump(hh\objprof_get_data());
    echo "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
  }
}


<<__EntryPoint>>
function main_from_obj_dtor() {
  error_reporting(E_ERROR);
  test();
}

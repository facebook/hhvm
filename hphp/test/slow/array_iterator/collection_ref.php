<?hh
function test(Vector $vec, &$val) {
  foreach ($vec as $val) {
    var_dump($val);
  }
}


<<__EntryPoint>>
function main_collection_ref() {
  test(Vector { 1,2,3 }, &$val);
}

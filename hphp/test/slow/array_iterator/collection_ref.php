<?hh
function test(Vector $vec) {
  $a = 0;
  if ($_ENV['HHVM_ARCH'] == 'x64') $val = &$a;
  foreach ($vec as $val) {
    var_dump($val);
  }
}


<<__EntryPoint>>
function main_collection_ref() {
test(Vector { 1,2,3 });
}

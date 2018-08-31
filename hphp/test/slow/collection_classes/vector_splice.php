<?hh

function test($v, $offset, $len) {
  echo "$offset $len: ";
  $v->splice($offset, $len);
  echo implode(' ', $v) . "\n";
}

function main() {
  foreach (range(-4, 4) as $offset) {
    foreach (range(-4, 4) as $len) {
      test(Vector {1, 2, 3}, $offset, $len);
    }
  }
}


<<__EntryPoint>>
function main_vector_splice() {
main();
}

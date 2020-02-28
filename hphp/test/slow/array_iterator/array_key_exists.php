<?hh

function test($x) {
  // these never exist with our inputs
  var_dump(array_key_exists('storage', $x));
  var_dump(array_key_exists("\0" . get_class($x) . "\0storage", $x));
  var_dump(array_key_exists('wat', $x));
  // exactly one of these should exist with our inputs
  var_dump(array_key_exists('a', $x));
  var_dump(array_key_exists('p', $x));
}

<<__EntryPoint>>
function main() {
  $x = new ArrayIterator(darray['a' => 'b']);
  $x->p = 'q';
  test($x);
  echo "----\n";
  $x = new ArrayIterator(darray['a' => 'b'],  ArrayIterator::STD_PROP_LIST);
  $x->p = 'q';
  test($x);
}

<?hh

function test($name, $a) {
  echo "---- $name\n";
  var_dump(Vector::fromArray($a));
}

function main() {
  test('empty', varray[]);
  test('packed', varray[1, 2, 3]);
  test('mixed', darray['a' => 'b', 'c' => 'd']);
}


<<__EntryPoint>>
function main_vector_from_array() {
main();
}

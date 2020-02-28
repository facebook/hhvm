<?hh

function main() {
  $x = varray[];
  $x[42] = 2;
  $x[] = 3;
  var_dump($x);

  $x = varray[];
  $x[PHP_INT_MAX] = 2;
  $x[] = 3;
  var_dump($x);
}


<<__EntryPoint>>
function main_empty_array_010() {
main();
}

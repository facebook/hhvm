<?hh

// Test creating a FrozenVector literal.

function main() {
  $fv = FrozenVector {"hello", "world"};
  var_dump($fv->get(0) . ' ' . $fv->get(1));
}

main();

<?hh

// Test creating a FixedVector literal.

function main() {
  $fv = FixedVector {"hello", "world"};
  var_dump($fv->get(0) . ' ' . $fv->get(1));
}

main();

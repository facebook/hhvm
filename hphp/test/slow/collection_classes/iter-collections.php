<?hh

// Test that iterators for Vector and Map remain valid
// if an existing element is overwritten.

function main() {

  // Vector
  $v = Vector {0, 0, 0};
  foreach ($v as $e) {
    $v[0] = 42; // the "current" element as seen by the iterator is unmodified
    $v[2] = 42; // but "future" changes are seen
    var_dump($e);
  }
  var_dump($v);

  // Map
  $sm = Map {13 => 0, 24 => 0};
  foreach ($sm as $k => $v) {
    $sm[$k] = 42;
  }
  var_dump($sm);
}

main();

<?hh

// Test that iterators for Vector, Map, and StableMap  remain valid
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
  $m = Map {10 => 20};
  foreach ($m as $k => $v) {
    $m[$k] = 42;
  }
  var_dump($m);

  // StableMap
  $sm = StableMap {13 => 0, 24 => 0};
  foreach ($sm as $k => $v) {
    $sm[$k] = 42;
  }
  var_dump($sm);
}

main();

<?hh

function test_start_iteration($a) {
  echo "---- trying to start iterating:\n";
  var_dump($a);
  try {
    foreach ($a as &$v) {
      throw new Exception("TEST FAIL: should not get here 1");
    }
    throw new Exception("TEST FAIL: should not get here 2");
  } catch (Exception $e) {
    echo "Got Exception: " . $e->getMessage() . "\n";
  }
}

function test_swap_while_iterating($a) {
  echo "---- trying to switch arrays while iterating to:\n";
  var_dump($a);
  $arr = array(10, 11, 12);
  foreach ($arr as &$v) {
    echo "$v\n";
    if ($v == 11) {
      // switching to a hack array should stop iteration
      $arr = $a;
    }
  }
}

function test() {
  $hack_arrays = array(
    vec[1, 2, 3],
    dict['a' => 'b', 'c' => 'd'],
    keyset['a', 'b', 'c'],
  );
  foreach ($hack_arrays as $a) {
    test_start_iteration($a);
    test_swap_while_iterating($a);
  }
}
test();

<?hh
function main() {
  $containers = Vector {
    ImmVector {Vector {}, Vector {}},
    ImmMap {0 => Vector {}, 1 => Vector {}},
    Pair {Vector {}, Vector {}}
  };
  foreach ($containers as $x) {
    $x[0][] = 1;
    $x[1][] = 2;
    var_dump($x);
  }
  echo "\n";

  $containers = Vector {
    ImmVector {array(Vector {}), array(Vector {})},
    ImmMap {0 => array(Vector {}), 1 => array(Vector {})},
    Pair {array(Vector {}), array(Vector {})}
  };
  foreach ($containers as $x) {
    $y = $x[0][0];
    $y[] = 1;
    $y = $x[1][0];
    $y[] = 2;
    var_dump($x);
  }
  echo "\n";

  $containers = Vector {
    ImmVector {array(Vector {}), array(Vector {})},
    ImmMap {0 => array(Vector {}), 1 => array(Vector {})},
    Pair {array(Vector {}), array(Vector {})}
  };
  foreach ($containers as $x) {
    try {
      $x[0][0][] = 1;
      $x[1][0][] = 2;
      var_dump($x);
    } catch (InvalidOperationException $e) {
      echo "Caught exception: " . $e->getMessage() . "\n";
    }
  }
  echo "\n";

  $containers = Vector {
    ImmVector {'foo', true},
    ImmMap {0 => 'foo', 1 => true},
    Pair {'foo', true}
  };
  foreach ($containers as $x) {
    try {
      $x[0] .= 'baz';
      $x[1] += 456;
      var_dump($x);
    } catch (InvalidOperationException $e) {
      echo "Caught exception: " . $e->getMessage() . "\n";
    }
  }
  echo "\n";

  $containers = Vector {
    ImmVector {4, 6},
    ImmMap {0 => 4, 1 => 6},
    Pair {4, 6}
  };
  foreach ($containers as $x) {
    try {
      ++$x[0];
      $x[1]++;
      var_dump($x);
    } catch (InvalidOperationException $e) {
      echo "Caught exception: " . $e->getMessage() . "\n";
    }
  }
  echo "\n";

  $containers = Vector {
    ImmVector {null, false},
    ImmMap {0 => null, 1 => false},
    Pair {null, false}
  };
  foreach ($containers as $x) {
    try {
      $x[0][] = 73;
      $x[1]->prop = 'blah';
      var_dump($x);
    } catch (InvalidOperationException $e) {
      echo "Caught exception: " . $e->getMessage() . "\n";
    }
  }
  echo "\n";

  $containers = Vector {
    ImmVector {array(), array()},
    ImmMap {0 => array(), 1 => array()},
    Pair {array(), array()}
  };
  foreach ($containers as $x) {
    try {
      $x[0][] = 73;
      $x[1]['a'] = 'b';
      var_dump($x);
    } catch (InvalidOperationException $e) {
      echo "Caught exception: " . $e->getMessage() . "\n";
    }
  }
  echo "\n";

  $containers = Vector {
    ImmVector {'foo', 'bar'},
    ImmMap {0 => 'foo', 1 => 'bar'},
    Pair {'foo', 'bar'}
  };
  foreach ($containers as $x) {
    try {
      $x[0][0] = 'b';
      $x[1][0] = 'c';
      var_dump($x);
    } catch (InvalidOperationException $e) {
      echo "Caught exception: " . $e->getMessage() . "\n";
    }
  }
  echo "\n";

  $containers = Vector {
    ImmVector {array(4, 5, 6), array(7, 8, 9)},
    ImmMap {0 => array(4, 5, 6), 1 => array(7, 8, 9)},
    Pair {array(4, 5, 6), array(7, 8, 9)}
  };
  foreach ($containers as $x) {
    try {
      unset($x[0]);
      unset($x[1]);
      var_dump($x);
    } catch (InvalidOperationException $e) {
      echo "Caught exception: " . $e->getMessage() . "\n";
    }
  }
  echo "\n";

  $containers = Vector {
    ImmVector {array(4, 5, 6), array(7, 8, 9)},
    ImmMap {0 => array(4, 5, 6), 1 => array(7, 8, 9)},
    Pair {array(4, 5, 6), array(7, 8, 9)}
  };
  foreach ($containers as $x) {
    try {
      unset($x[0][0]);
      unset($x[1][0]);
      var_dump($x);
    } catch (InvalidOperationException $e) {
      echo "Caught exception: " . $e->getMessage() . "\n";
    }
  }
  echo "\n";

  $containers = Vector {
    ImmVector {array(), array()},
    ImmMap {0 => array(), 1 => array()},
    Pair {array(), array()}
  };
  foreach ($containers as $x) {
    try {
      $x[][1] = 1;
      $x[][1] = 2;
      var_dump($x);
    } catch (InvalidOperationException $e) {
      echo "Caught exception: " . $e->getMessage() . "\n";
    }
  }
  echo "\n";

  $x = Pair {array(1), array(2)};
  $y =& $x[0][0];
  $y = 2;
  var_dump($x);
}

main();


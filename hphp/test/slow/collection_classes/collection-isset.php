<?hh
function main() {
  $containers = Vector {
    Pair {Vector {}, Vector {Vector {}}},
    Pair {ImmVector {}, ImmVector {ImmVector {}}},
    Pair {Pair {42, 73}, Pair {Pair {43, 74}, Pair {44, 75}}}
  };
  foreach ($containers as list($x, $y)) {
    var_dump(isset($x[11][22]));
    var_dump(isset($x[11]['a']));
    try {
      var_dump(isset($x['a']['b']));
    } catch (InvalidArgumentException $e) {
      echo "Caught exception\n";
    }
    var_dump(isset($y[0][22]));
    try {
      var_dump(isset($y[0]['a']));
    } catch (InvalidArgumentException $e) {
      echo "Caught exception\n";
    }
    echo "\n";
  }

  $containers = Vector {
    Pair {Map {}, Map {'a' => Map {}}},
    Pair {ImmMap {}, ImmMap {'a' => ImmMap {}}}
  };
  foreach ($containers as list($x, $y)) {
    var_dump(isset($x[11][22]));
    var_dump(isset($x['a']['b']));
    try {
      var_dump(isset($x[null]['b']));
    } catch (InvalidArgumentException $e) {
      echo "Caught exception\n";
    }
    var_dump(isset($y['a']['b']));
    try {
      var_dump(isset($y['a'][null]));
    } catch (InvalidArgumentException $e) {
      echo "Caught exception\n";
    }
    echo "\n";
  }
}
main();

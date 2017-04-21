<?hh

class ToString {
  function __toString() {
    return "foobaz";
  }
}

class Noisy {
  public $id;
  function __construct($id) {
    $this->id = $id;
  }
  function __destruct() {
    echo $this->id . " Noisy::__destruct()\n";
  }
}

class Thrower {
  function __construct() {
    throw new Exception("Thrower::__construct()");
  }
}

function create($a, $b, $c, $d) {
  try {
    var_dump(vec[$a, $b, $c, $d]);
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

function main() {
  // These should succeed:
  var_dump(vec[]);
  var_dump(vec['a', 'b', 'c']);
  var_dump(vec[1, 2, 3]);
  var_dump(vec[1, 'b', 3]);
  var_dump(vec[10, 'abc', 10, 'abc']);
  var_dump(vec[123, '123']);
  var_dump(vec[null, false, true, 7.89, [], vec[], keyset[], dict[]]);
  var_dump(vec[new stdclass, 1234, 'abc', Vector{1, 2, 3}]);

  create('a', 'b', 'c', 'd');
  create(1, 2, 3, 4);
  create(1, 'a', 2, 'b');
  create(10, 10, 'abc', 'abc');
  create(123, '123', 123, '123');
  create(1.23, null, false, true);
  create(new stdclass, [], vec[1, 2, 3], keyset['a', 1]);
  create(dict[1 => 'a'], new ToString, 'abc', 10, Vector{1, 2, 3});

  try {
    var_dump(vec[new Noisy(1), new Noisy(2),
                 new Noisy(3), new Thrower,
                 new Noisy(4)]);
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

main();

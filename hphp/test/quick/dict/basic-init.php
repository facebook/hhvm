<?php

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

function create($a, $b, $c, $d) {
  try {
    var_dump(dict[$a => $b, $c => $d]);
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

function create_noisy($a, $b, $c, $d) {
  try {
    var_dump(dict[$a => new Noisy($c), $b => new Noisy($d)]);
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

function main() {
  // These should succeed:
  var_dump(dict[]);
  var_dump(dict['a' => 1, 'b' => 2, 'c' => 3]);
  var_dump(dict[1 => 'a', 2 => 'b', 3 => 'c']);
  var_dump(dict[1 => 'a', 'b' => 2, 3 => 'c']);
  var_dump(dict[10 => 'abc', 10 => 'abc', 10 => 'abc']);
  var_dump(dict['abc' => 10, 'abc' => 10, 'abc' => 10]);
  var_dump(dict[123 => 100, '123' => 200]);
  var_dump(dict[1 => true, 2 => false, 3 => 1.23,
                4 => null, 5 => [], 6 => vec[1, 2, 3],
                7 => dict[1 => 'a'], 8 => keyset['a', 'b']]);
  var_dump(dict['abc' => new stdclass]);

  create('a', 1, 'b', 2);
  create(1, 'a', 2, 'b');
  create(1, 'a', 'b', 2);
  create(10, 'abc', 10, 'def');
  create('abc', 10, 'abc', 20);
  create('123', 456, 123, '456');
  create(100, true, 200, false);
  create('abc', new stdclass, 'def', new stdclass);
  create(100, vec[1, 2, 3], 200, vec[4, 5, 6]);
  create('abc', null, 'def', 4.56);
  create_noisy('abc', 'abc', 1, 2);

  // These should fail:
  create(null, 'a', null, 'b');
  create(false, 1, true, 2);
  create(1.23, 'a', 4.56, 'b');
  create([], 'a', [1, 2, 3], 'b');
  create(vec[2, 1], 100, vec[], 200);
  create(dict['a' => 1], 100, dict[], 200);
  create(keyset[], 'a', keyset['a'], 'b');
  create(new stdclass, 'a', new stdclass, 'b');
  create(new ToString, 1, new ToString, 2);
  create(Vector{1, 2, 3}, 123, Vector{4, 5, 6}, 456);
  create(false, 'first', null, 'second');
  create(vec[], 'first', dict[], 'second');
  create_noisy(1, [], 3, 4);
  create_noisy(false, true, 5, 6);
}

main();

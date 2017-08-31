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

function create($a, $b, $c) {
  try {
    var_dump(keyset[$a, $b, $c]);
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

function main() {
  // These should succeed:
  var_dump(keyset[]);
  var_dump(keyset['a', 'b', 'c']);
  var_dump(keyset[1, 2, 3]);
  var_dump(keyset[1, 'b', 3]);
  var_dump(keyset[10, 10, 10]);
  var_dump(keyset['abc', 'abc', 'abc']);
  var_dump(keyset[123, '123']);

  create('a', 'b', 'c');
  create(1, 2, 3);
  create(1, 'b', 3);
  create(10, 10, 10);
  create('abc', 'abc', 'abc');
  create('123', 456, 123);

  // These should fail:
  create('a', null, 'b');
  create(1, 2, false);
  create(1, 2, 3.14);
  create('a', 'b', []);
  create(4, 3, vec[2, 1]);
  create(dict['a' => 1], 'a', 5);
  create(100, keyset['a', 'b'], 200);
  create('a', new stdclass, 'c');
  create(1, Vector{1, 2, 3}, 2);
  create('a', new ToString, 'c');

  try {
    var_dump(keyset['a', 1, 'b', 2, new Noisy(1), new Noisy(2)]);
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

main();

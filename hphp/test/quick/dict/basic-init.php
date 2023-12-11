<?hh

class ToString {
  function __toString() :mixed{
    return "foobaz";
  }
}

function create($a, $b, $c, $d) :mixed{
  try {
    var_dump(dict[$a => $b, $c => $d]);
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

<<__EntryPoint>> function main(): void {
  // These should succeed:
  var_dump(dict[]);
  var_dump(dict['a' => 1, 'b' => 2, 'c' => 3]);
  var_dump(dict[1 => 'a', 2 => 'b', 3 => 'c']);
  var_dump(dict[1 => 'a', 'b' => 2, 3 => 'c']);
  var_dump(dict[10 => 'abc', 10 => 'abc', 10 => 'abc']);
  var_dump(dict['abc' => 10, 'abc' => 10, 'abc' => 10]);
  var_dump(dict[123 => 100, '123' => 200]);
  var_dump(dict[1 => true, 2 => false, 3 => 1.23,
                4 => null, 5 => vec[], 6 => vec[1, 2, 3],
                7 => dict[1 => 'a'], 8 => keyset['a', 'b']]);
  var_dump(dict['abc' => new stdClass]);

  create('a', 1, 'b', 2);
  create(1, 'a', 2, 'b');
  create(1, 'a', 'b', 2);
  create(10, 'abc', 10, 'def');
  create('abc', 10, 'abc', 20);
  create('123', 456, 123, '456');
  create(100, true, 200, false);
  create('abc', new stdClass, 'def', new stdClass);
  create(100, vec[1, 2, 3], 200, vec[4, 5, 6]);
  create('abc', null, 'def', 4.56);

  // These should fail:
  create(null, 'a', null, 'b');
  create(false, 1, true, 2);
  create(1.23, 'a', 4.56, 'b');
  create(vec[], 'a', vec[1, 2, 3], 'b');
  create(vec[2, 1], 100, vec[], 200);
  create(dict['a' => 1], 100, dict[], 200);
  create(keyset[], 'a', keyset['a'], 'b');
  create(new stdClass, 'a', new stdClass, 'b');
  create(new ToString, 1, new ToString, 2);
  create(Vector{1, 2, 3}, 123, Vector{4, 5, 6}, 456);
  create(false, 'first', null, 'second');
  create(vec[], 'first', dict[], 'second');
}

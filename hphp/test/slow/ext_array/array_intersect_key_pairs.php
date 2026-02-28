<?hh

function test_left($x) :mixed{
  echo "-- left pair\n";
  var_dump($x, array_intersect_key(Pair{'a', 'b'}, $x));
}

function test_right($x) :mixed{
  echo "-- right pair\n";
  var_dump($x, array_intersect_key($x, Pair{'a', 'b'}));
}

<<__EntryPoint>>
function test() :mixed{
  test_left(dict['0' => 'x', '1' => 'y', '2' => 'z']);
  test_left(dict[0 => 'x', 1 => 'y', 2 => 'z']);
  test_left(Map{'0' => 'x', '1' => 'y', '2' => 'z'});
  test_left(Map{0 => 'x', 1 => 'y', 2 => 'z'});

  test_right(dict['0' => 'x', '1' => 'y', '2' => 'z']);
  test_right(dict[0 => 'x', 1 => 'y', 2 => 'z']);
  test_right(Map{'0' => 'x', '1' => 'y', '2' => 'z'});
  test_right(Map{0 => 'x', 1 => 'y', 2 => 'z'});
}

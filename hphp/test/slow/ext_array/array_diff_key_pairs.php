<?hh

function test_left($x) {
  echo "-- left pair\n";
  var_dump($x, array_diff_key(Pair{'a', 'b'}, $x));
}

function test_right($x) {
  echo "-- right pair\n";
  var_dump($x, array_diff_key($x, Pair{'a', 'b'}));
}

<<__EntryPoint>>
function test() {
  test_left(dict['0' => 'x', '1' => 'y', '2' => 'z']);
  test_left(dict[0 => 'x', 1 => 'y', 2 => 'z']);
  test_left(Map{'0' => 'x', '1' => 'y', '2' => 'z'});
  test_left(Map{0 => 'x', 1 => 'y', 2 => 'z'});

  test_right(dict['0' => 'x', '1' => 'y', '2' => 'z']);
  test_right(dict[0 => 'x', 1 => 'y', 2 => 'z']);
  test_right(Map{'0' => 'x', '1' => 'y', '2' => 'z'});
  test_right(Map{0 => 'x', 1 => 'y', 2 => 'z'});
}

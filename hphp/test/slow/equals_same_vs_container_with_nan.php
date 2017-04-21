<?hh

function compare($type, $c, $d) {
  echo "--- $type ----\n";
  var_dump($c === $c);
  var_dump($c == $c);
  var_dump($c === $d);
  var_dump($c == $d);
}

function test_veclike($name, $c, $d) {
  // we take partly filled containers as an arguments and mutate them before
  // passing them to compare, so that interning and hhbbc can't make them be
  // backed by the same underlying storage; if that happened the assumptions
  // of this test would be violated
  $c[] = NAN;
  $d[] = NAN;
  compare($name, $c, $d);
}

function test_dictlike($name, $c, $d) {
  // we take partly filled containers as an arguments and mutate them before
  // passing them to compare, so that interning and hhbbc can't make them be
  // backed by the same underlying storage; if that happened the assumptions
  // of this test would be violated
  $c['b'] = NAN;
  $d['b'] = NAN;
  compare($name, $c, $d);
}

function test() {
  test_veclike('Packed Array', [1], [1]);
  test_dictlike('Mixed Array', ['a' => 1], ['a' => 1]);

  test_veclike('Vec', vec[1], vec[1]);
  test_dictlike('Dict', dict['a' => 1], dict['a' => 1]);

  test_veclike('Vector', Vector {1}, Vector {1});
  test_dictlike('Map', Map {'a' => 1}, Map {'a' => 1});
}

test();

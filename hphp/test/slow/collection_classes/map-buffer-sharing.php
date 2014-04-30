<?hh
function main() {
  $x = Map {0 => 1};
  $y = $x->toImmMap();
  $x[0] += 2;
  var_dump($y);
  unset($y);
  unset($x);

  $x = Map {0 => 1};
  $y = $x->toImmMap();
  ++$x[0];
  var_dump($y);
  unset($y);
  unset($x);

  $x = Map {0 => null};
  $y = $x->toImmMap();
  $x[0][] = 73;
  var_dump($y);
  unset($y);
  unset($x);

  $x = Map {0 => null};
  $y = $x->toImmMap();
  $x[0][42] = 73;
  var_dump($y);
  unset($y);
  unset($x);

  $x = Map {0 => null};
  $y = $x->toImmMap();
  $x[0]->prop = 73;
  var_dump($y);
  $x[0] = null;
  unset($y);
  unset($x);

  $x = Map {0 => 'foo'};
  $y = $x->toImmMap();
  $x[0][0] = 'g';
  var_dump($y);
  unset($y);
  unset($x);

  $x = Map {0 => array(1)};
  $y = $x->toImmMap();
  unset($x[0][0]);
  var_dump($y);
  unset($y);
  unset($x);

  $x = Map {0 => array(1)};
  $y = $x->toImmMap();
  $x[0][] = 2;
  var_dump($y);
  unset($y);
  unset($x);

  $x = Map {0 => array(1)};
  $y = $x->toImmMap();
  $x[0][1] = 2;
  var_dump($y);
  unset($y);
  unset($x);

  $x = Map {0 => Map {0 => 1}};
  $y = $x->toImmMap();
  ++$x[0][0];
  var_dump($y);
  unset($y);
  $x[1] = null;
  $x->clear();
  unset($x);

  $x = Map {0 => Map {0 => 1}};
  $y = $x->toImmMap();
  $x[0][0] += 2;
  var_dump($y);
  unset($y);
  $x[1] = null;
  $x->clear();
  unset($x);

  $x = Map {0 => Map {0 => 1}};
  $y = $x->toImmMap();
  $x[0][] = Pair {1, 2};
  var_dump($y);
  unset($y);
  $x[1] = null;
  $x->clear();
  unset($x);

  $x = Map {0 => Map {0 => 1}};
  $y = $x->toImmMap();
  $x[0][0] = 2;
  var_dump($y);
  unset($y);
  $x[1] = null;
  $x->clear();
  unset($x);
}
main();

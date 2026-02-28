<?hh
function main() :mixed{
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
  $x[0] = vec[]; $x[0][] = 73;
  var_dump($y);
  unset($y);
  unset($x);

  $x = Map {0 => null};
  $y = $x->toImmMap();
  $x[0] = dict[]; $x[0][42] = 73;
  var_dump($y);
  unset($y);
  unset($x);

  $x = Map {0 => new stdClass()};
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

  $x = Map {0 => vec[1]};
  $y = $x->toImmMap();
  unset($x[0][0]);
  var_dump($y);
  unset($y);
  unset($x);

  $x = Map {0 => vec[1]};
  $y = $x->toImmMap();
  $x[0][] = 2;
  var_dump($y);
  unset($y);
  unset($x);

  $x = Map {0 => dict[0 => 1]};
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
  $x[0]->add(Pair {1, 2});
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

<<__EntryPoint>>
function main_map_buffer_sharing() :mixed{
main();
}

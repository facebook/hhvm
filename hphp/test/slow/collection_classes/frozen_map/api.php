<?hh

// Test FrozenMap's PHP-accessible public methods.

// Helpers
function show_keyed_iter($iter) {
  $vs = Vector {};

  foreach ($iter as $k => $v) {
    $vs[] = Pair {$k, $v};
  }

  usort($vs, function($p1, $p2) {
    if ($p1[0] < $p2[0]) return -1;
    else if ($p1[0] == $p2[0]) return 0;
    else return 1;
  });

  echo get_class($iter), " [\n";
  foreach ($vs as $v) var_dump($v);
  echo "]\n";
}

function show_iter($iter) {
  $vs = new Vector($iter);
  sort($vs);

  echo get_class($iter), " [\n";
  foreach ($vs as $v) var_dump($v);
  echo "]\n";
}
/////////////////////////////////////

function main() {
  $fm = new FrozenMap(Vector {1, 2, 3});
  $e = new FrozenMap();

  echo "\n= empty =\n";
  var_dump($fm->isEmpty());
  var_dump($e->isEmpty());

  echo "\n= count =\n";
  var_dump($fm->count());
  var_dump($e->count());

  echo "\n= items =\n";
  var_dump($fm->items()->toVector()->count());

  echo "\n= keys =\n";
  show_iter($fm->keys());

  echo "\n= lazy =\n";
  show_keyed_iter($fm->lazy());

  echo "\n= at =\n";
  var_dump($fm->at(0));

  try {
    var_dump($fm->at(3));
  } catch (Exception $ex) {
    echo get_class($ex), ": ", $ex->getMessage(), "\n";
  }
  try {
    var_dump($fm->at('3'));
  } catch (Exception $ex) {
    echo get_class($ex), ": ", $ex->getMessage(), "\n";
  }

  echo "\n= get =\n";
  var_dump($fm->get(0));
  var_dump($fm->get(3));

  echo "\n= contains =\n";
  var_dump($fm->contains(1));
  var_dump($fm->contains(3));
  var_dump($e->contains(0));

  echo "\n= containskey..\n";
  var_dump($fm->containsKey(1));
  var_dump($fm->containsKey(3));
  var_dump($e->containsKey(0));

  echo "\n= values =\n";
  show_iter($fm->values());

  echo "\n= differencebykey =\n";
  $m1 = new FrozenMap(Map {1 => 1, 2 => 2, 3 => 3});
  $m2 = new FrozenMap(Map {1 => 1, 2 => 2});
  $res =$m1->differenceByKey($m2);
  var_dump($res instanceof FrozenMap);
  show_keyed_iter($res);

  echo "\n= map =\n";
  $res = $fm->map(function ($v) { return 2 * $v; });
  var_dump($res instanceof FrozenMap);
  show_keyed_iter($res);

  echo "\n= mapwithkey =\n";
  $res = $fm->mapWithKey(function ($k, $v) { return Pair {$k, $v}; });
  var_dump($res instanceof FrozenMap);
  show_keyed_iter($res);

  echo "\n= filter =\n";
  $res = $fm->filter(function ($v) { return $v == 1; });
  var_dump($res instanceof FrozenMap);
  show_keyed_iter($res);

  echo "\n= filterwithkey =\n";
  $res = $fm->filterWithKey(function ($k, $v) { return $k == 0 || $v == 3; });
  var_dump($res instanceof FrozenMap);
  show_keyed_iter($res);

  echo "\n= fromItems =\n";
  var_dump(FrozenMap::fromItems(Vector {}) instanceof FrozenMap);
  show_keyed_iter(FrozenMap::fromItems((Vector {Pair {1, 2}, Pair {3, 4}, Pair {5, 6}})));
}

main();

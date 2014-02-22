<?hh

function print_keyed_iterable($iterable) {
  echo get_class($iterable) . "[\n";
  foreach ($iterable as $k => $v) {
    echo $k . " => " . $v . "\n";
  }
  echo "]\n";
}

function main() {
  echo "******** Vector ********\n";
  $x = Vector {1, 2, 3, 4};
  $y = $x->mapWithKey(function ($k,$v) {
      echo "callback($k, $v)\n";
      return $v + 100;
    });
  echo "========\n";
  print_keyed_iterable($y);
  echo "========\n";
  $x = Vector {1, 2, 3, 4};
  $y = $x->filterWithKey(function ($k,$v) {
      echo "callback($k, $v)\n";
      return (bool)($v % 2);
    });
  echo "========\n";
  print_keyed_iterable($y);

  echo "******** Map ********\n";

  $x = Map {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4};
  $y = $x->mapWithKey(function ($k,$v) {
      echo "callback($k, $v)\n";
      return $v + 100;
    });
  echo "========\n";
  print_keyed_iterable($y);
  echo "========\n";
  $x = Map {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4};
  $y = $x->filterWithKey(function ($k,$v) {
      echo "callback($k, $v)\n";
      return (bool)($v % 2);
    });
  echo "========\n";
  print_keyed_iterable($y);
  echo "========\n";
  $x = Map {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4};
  $y = $x->retainWithKey(function ($k,$v) {
      echo "callback($k, $v)\n";
      return (bool)($v % 2);
    });
  echo "========\n";
  var_dump($x === $y);
  print_keyed_iterable($y);

  echo "******** Pair ********\n";

  $x = Pair {1, 2};
  $y = $x->mapWithKey(function ($k,$v) {
      echo "callback($k, $v)\n";
      return $v + 100;
    });
  echo "========\n";
  print_keyed_iterable($y);
  echo "========\n";
  $x = Pair {1, 2};
  $y = $x->filterWithKey(function ($k,$v) {
      echo "callback($k, $v)\n";
      return (bool)($v % 2);
    });
  echo "========\n";
  print_keyed_iterable($y);

  echo "\n\n\n\n";

  echo "******** LazyVector ********\n";

  $x = Vector {1, 2, 3, 4};
  $y = $x->lazy()->mapWithKey(function ($k,$v) {
      echo "callback($k, $v)\n";
      return $v + 100;
    });
  echo "========\n";
  print_keyed_iterable($y);
  echo "========\n";
  $x = Vector {1, 2, 3, 4};
  $y = $x->lazy()->filterWithKey(function ($k,$v) {
      echo "callback($k, $v)\n";
      return (bool)($v % 2);
    });
  echo "========\n";
  print_keyed_iterable($y);

  echo "******** LazyMap ********\n";

  $x = Map {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4};
  $y = $x->lazy()->mapWithKey(function ($k,$v) {
      echo "callback($k, $v)\n";
      return $v + 100;
    });
  echo "========\n";
  foreach ($y as $k => $v) {
    echo $k . " => " . $v . "\n";
  }
  echo "========\n";
  $x = Map {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4};
  $y = $x->lazy()->filterWithKey(function ($k,$v) {
      echo "callback($k, $v)\n";
      return (bool)($v % 2);
    });
  echo "========\n";
  print_keyed_iterable($y);

  echo "******** LazyPair ********\n";

  $x = Pair {1, 2};
  $y = $x->lazy()->mapWithKey(function ($k,$v) {
      echo "callback($k, $v)\n";
      return $v + 100;
    });
  echo "========\n";
  print_keyed_iterable($y);
  echo "========\n";
  $x = Pair {1, 2};
  $y = $x->lazy()->filterWithKey(function ($k,$v) {
      echo "callback($k, $v)\n";
      return (bool)($v % 2);
    });
  echo "========\n";
  print_keyed_iterable($y);
}
main();

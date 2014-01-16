<?hh

$x = Vector {1, 2, 3, 4};
$y = $x->mapWithKey(function ($k,$v) {
  echo "callback($k, $v)\n";
  return $v + 100;
});
echo "========\n";
echo get_class($y) . "\n";
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}
echo "========\n";
$x = Vector {1, 2, 3, 4};
$y = $x->filterWithKey(function ($k,$v) {
  echo "callback($k, $v)\n";
  return (bool)($v % 2);
});
echo "========\n";
echo get_class($y) . "\n";
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}

echo "********\n";

$x = Map {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4};
$y = $x->mapWithKey(function ($k,$v) {
  echo "callback($k, $v)\n";
  return $v + 100;
});
echo "========\n";
echo get_class($y) . "\n";
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}
echo "========\n";
$x = Map {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4};
$y = $x->filterWithKey(function ($k,$v) {
  echo "callback($k, $v)\n";
  return (bool)($v % 2);
});
echo "========\n";
echo get_class($y) . "\n";
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}

echo "********\n";

$x = StableMap {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4};
$y = $x->mapWithKey(function ($k,$v) {
  echo "callback($k, $v)\n";
  return $v + 100;
});
echo "========\n";
echo get_class($y) . "\n";
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}
echo "========\n";
$x = StableMap {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4};
$y = $x->filterWithKey(function ($k,$v) {
  echo "callback($k, $v)\n";
  return (bool)($v % 2);
});
echo "========\n";
echo get_class($y) . "\n";
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}

echo "********\n";

$x = Pair {1, 2};
$y = $x->mapWithKey(function ($k,$v) {
  echo "callback($k, $v)\n";
  return $v + 100;
});
echo "========\n";
echo get_class($y) . "\n";
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}
echo "========\n";
$x = Pair {1, 2};
$y = $x->filterWithKey(function ($k,$v) {
  echo "callback($k, $v)\n";
  return (bool)($v % 2);
});
echo "========\n";
echo get_class($y) . "\n";
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}

echo "\n\n********\n\n\n";

$x = Vector {1, 2, 3, 4};
$y = $x->lazy()->mapWithKey(function ($k,$v) {
  echo "callback($k, $v)\n";
  return $v + 100;
});
echo "========\n";
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}
echo "========\n";
$x = Vector {1, 2, 3, 4};
$y = $x->lazy()->filterWithKey(function ($k,$v) {
  echo "callback($k, $v)\n";
  return (bool)($v % 2);
});
echo "========\n";
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}

echo "********\n";

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
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}

echo "********\n";

$x = StableMap {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4};
$y = $x->lazy()->mapWithKey(function ($k,$v) {
  echo "callback($k, $v)\n";
  return $v + 100;
});
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}
echo "========\n";
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}
echo "========\n";
$x = StableMap {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4};
$y = $x->lazy()->filterWithKey(function ($k,$v) {
  echo "callback($k, $v)\n";
  return (bool)($v % 2);
});
echo "========\n";
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}

echo "********\n";

$x = Pair {1, 2};
$y = $x->lazy()->mapWithKey(function ($k,$v) {
  echo "callback($k, $v)\n";
  return $v + 100;
});
echo "========\n";
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}
echo "========\n";
$x = Pair {1, 2};
$y = $x->lazy()->filterWithKey(function ($k,$v) {
  echo "callback($k, $v)\n";
  return (bool)($v % 2);
});
echo "========\n";
foreach ($y as $k => $v) {
  echo $k . " => " . $v . "\n";
}


<?hh

function print_keyed_iterable($iterable) {
  echo get_class($iterable) . "[\n";
  foreach ($iterable as $k => $v) {
    echo $k . " => " . $v . "\n";
  }
  echo "]\n";
}

function main() {
  echo "******** Map ********\n";

  $x = Map {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4,};
  $y = $x->retain(function ($v) {
      echo "callback($v)\n";
      return true;
    });
  echo "========\n";
  var_dump($x === $y);
  print_keyed_iterable($y);
  echo "========\n";
  $x = Map {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4,};
  $y = $x->retain(function ($v) {
      echo "callback($v)\n";
      return (bool)($v % 2);
    });
  echo "========\n";
  var_dump($x === $y);
  print_keyed_iterable($y);
}
main();

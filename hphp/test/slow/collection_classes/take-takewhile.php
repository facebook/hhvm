<?hh
function main() {
  $containers = Vector {
    Vector {1, 2, 3, 4},
    ImmVector {1, 2, 3, 4},
    Map {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4},
    ImmMap {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4},
    Set {1, 2, 3, 4},
    ImmSet {1, 2, 3, 4},
    Pair {1, 2},
    (Set {1, 2, 3, 4})->lazy(),
    (Map {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4})->lazy(),
  };
  foreach ($containers as $c) {
    $name = get_class($c);
    echo "$name\n";
    foreach (array(~PHP_INT_MAX, -1, 0, 1, 2, 6, PHP_INT_MAX) as $n) {
      if ($name === Set::class ||
          $name === ImmSet::class ||
          $name === LazyIterableView::class) {
        echo "--------\n";
        echo "n = $n\n";
        foreach ($c->lazy()->take($n) as $v) {
          echo "$v\n";
        }
        echo "--------\n";
        echo "n = $n\n";
        foreach ($c->lazy()->takeWhile($x ==> $x <= $n) as $v) {
          echo "$v\n";
        }
      } else {
        echo "--------\n";
        echo "n = $n\n";
        foreach ($c->lazy()->take($n) as $k => $v) {
          echo "$k => $v\n";
        }
        echo "--------\n";
        echo "n = $n\n";
        foreach ($c->lazy()->takeWhile($x ==> $x <= $n) as $k => $v) {
          echo "$k => $v\n";
        }
      }
    }
    echo "\n";
  }
}
main();


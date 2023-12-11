<?hh

function main() :mixed{
  $containers = Vector {
    Vector {0, 1, 3, -2, 7, 4},
    ImmVector {0, 1, 3, -2, 7, 4},
    Map {'a' => 0, 'b' => 1, 'c' => 3, 'd' => -2, 'e' => 7, 'f' => 4},
    ImmMap {'a' => 0, 'b' => 1, 'c' => 3, 'd' => -2, 'e' => 7, 'f' => 4},
    Set {0, 1, 3, -2, 7, 4},
    ImmSet {0, 1, 3, -2, 7, 4},
  };
  foreach ($containers as $c) {
    $name = get_class($c);
    echo "$name\n";
    foreach (vec[~PHP_INT_MAX, -1, 0, 1, 2, 6, PHP_INT_MAX] as $n) {
      if ($name === Set::class || $name === ImmSet::class) {
        echo "--------\n";
        echo "n = $n\n";
        foreach ($c->skipWhile($x ==> $x <= $n) as $v) {
          echo "$v\n";
        }
      } else {
        echo "--------\n";
        echo "n = $n\n";
        foreach ($c->skipWhile($x ==> $x <= $n) as $k => $v) {
          echo "$k => $v\n";
        }
      }
    }
    echo "\n";
  }
}


<<__EntryPoint>>
function main_skipwhile() :mixed{
main();
}

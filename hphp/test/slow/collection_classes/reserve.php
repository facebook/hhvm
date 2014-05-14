<?hh

function try_reserve(Collection $c, int $sz) {
  try {
    $c->reserve($sz);
    echo 'Reserved at least ', $sz, ' for a ',
      get_class($c), ' of size ', $c->count(), "\n";
  } catch (Exception $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
}

function reserve_cases(Collection $c) {
  echo '------------------------------------------------------------', "\n";
  try_reserve($c, -1);
  try_reserve($c, 3);
  try_reserve($c, PHP_INT_MAX);
  try_reserve($c, 100);
  try_reserve($c, 15);
}

function main() {
  reserve_cases(Set {});
  reserve_cases(Map {});
  reserve_cases(Vector {});

  reserve_cases(Set {'a', 'b', 'c', 'd', 'e', 'f'});
  reserve_cases(Map {'a' => 1, 'b' => 2, 'c' => 3,
                     'd' => 4, 'e' => 5, 'f' => 6});
  reserve_cases(Vector {'a', 'b', 'c', 'd', 'e', 'f'});

}

main();

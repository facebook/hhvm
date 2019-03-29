<?php
function from($levels) {
  foreach (range(0, 2 << $levels) as $v) {
    yield $v;
  }
}
function gen($gen, $level) {
  if ($level % 2) {
    yield $gen->current();
  }
  yield from $gen;
}

<<__EntryPoint>>
function main_yield_from_multi_tree() {
  foreach (range(0, 6) as $levels) {
    print "$levels level".($levels == 1 ? "" : "s")."\n\n";
    $all = array();
    $gens = array(array());
    $all[] = $gens[0][0] = from($levels);
    for ($level = 1; $level < $levels; $level++) {
      for ($i = 0; $i < (1 << $level); $i++) {
        if (!array_key_exists($level, $gens)) $gens[$level] = array();
        $all[] = $gens[$level][$i] = gen($gens[$level-1][$i >> 1], $level);
      }
    }
    $first = true;
    while (1) {
      foreach ($all as $gen) {
        if ($first) {
          $gen->next();
        }
        var_dump($gen->current());
        $gen->next();
        if (!$gen->valid()) {
          break 2;
        }
      }
      $first = false;
    }
    print "\n\n";
  }
}

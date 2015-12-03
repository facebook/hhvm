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
foreach (range(0, 6) as $levels) {
  print "$levels level".($levels == 1 ? "" : "s")."\n\n";
  $all = array();
  $all[] = $gens[0][0] = from($levels);
  for ($level = 1; $level < $levels; $level++) {
    for ($i = 0; $i < (1 << $level); $i++) {
      $all[] = $gens[$level][$i] = gen($gens[$level-1][$i >> 1], $level);
    }
  }
  while (1) {
    foreach ($all as $gen) {
      var_dump($gen->current());
      $gen->next();
      if (!$gen->valid()) {
        break 2;
      }
    }
  }
  print "\n\n";
}

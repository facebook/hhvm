<?php
function it() {
  yield from [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
}

function bar($gen) {
  $x = yield from $gen;
  echo "Return from bar: ";
  var_dump($x);
}

/* Twice a Generator from bar() using yield from on $gen */
$gen = it();
$gens[] = bar($gen);
$gens[] = bar($gen);

do {
  foreach ($gens as $g) {
    var_dump($g->current());
    $gen->next();
  }
} while($gen->valid());

foreach($gens as $g) {
  var_dump($g->valid());
  var_dump($g->current());
}

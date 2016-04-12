<?php
function gen($a = 0) {
  yield 1 + $a;
  if ($a < 1) {
    // TODO: HHVM currently does not support yield from (or yield) as
    // expressions. As such, this test had to be slightly modified.
    // The original line was:
    // var_dump(yield from gen($a + 1));
    $b = yield from gen($a + 1);
    var_dump($b);
  }
  yield 3 + $a;
  return 5 + $a;
}

function bar($gen) {
  // TODO: HHVM currently does not support yield from (or yield) as
  // expressions. As such, this test had to be slightly modified.
  // The original line was:
  // var_dump(yield from gen($a + 1));
  $x = yield from $gen;
  echo "Return from bar: ";
  var_dump($x);
}

/* Twice a Generator from bar() using yield from on $gen */
$gen = gen();
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

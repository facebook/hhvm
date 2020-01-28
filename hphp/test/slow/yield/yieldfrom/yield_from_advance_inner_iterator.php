<?hh
function it() {
  yield from varray[1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
}

function bar($gen) {
  $x = yield from $gen;
  echo "Return from bar: ";
  var_dump($x);
}


/* Twice a Generator from bar() using yield from on $gen */
<<__EntryPoint>>
function main_yield_from_advance_inner_iterator() {
  $gen = it();
  $bar1 = bar($gen);
  $bar2 = bar($gen);
  $bar1->next();
  $bar2->next();
  $gens = vec[$bar1, $bar2];

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
}

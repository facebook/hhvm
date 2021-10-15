<?hh

function const_mutation_pair(Pair<string,int> $x) : void {
  /* HH_FIXME[4011] */
  $x[1] = 1;
}

function const_mutation_keyedcontainer(KeyedContainer<string,int> $x) : void {
  /* HH_FIXME[4011] */
  $x[1] = 1;
}

function const_mutation_anyarray(AnyArray<string,int> $x) : void {
  /* HH_FIXME[4011] */
  $x[1] = 1;
}

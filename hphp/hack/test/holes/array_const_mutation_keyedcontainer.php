<?hh

function array_const_mutation_keyedcontainer(KeyedContainer<arraykey,int> $x) : void {
  /* HH_FIXME[4011] */
  $x[1] = 2;
}

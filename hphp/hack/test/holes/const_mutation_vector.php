<?hh

function const_mutation_immvector(ImmVector<int> $x) : void {
  /* HH_FIXME[4011] */
  $x[0] = 1;
}

function const_mutation_constvector(ConstVector<int> $x) : void {
  /* HH_FIXME[4011] */
  $x[0] = 1;
}

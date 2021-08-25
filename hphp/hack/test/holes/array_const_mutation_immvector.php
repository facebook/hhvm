<?hh

function array_const_mutation_immvector(ImmVector<int> $x) : void {
  /* HH_FIXME[4011] */
  $x[false] = 1.0;
}

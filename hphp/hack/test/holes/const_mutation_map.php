<?hh

function const_mutation_immmap(ImmMap<string,int> $x) : void {
  /* HH_FIXME[4011] */
  $x['a'] = 1;
}

function const_mutation_constmap(ConstMap<string,int> $x) : void {
  /* HH_FIXME[4011] */
  $x['a'] = 1;
}

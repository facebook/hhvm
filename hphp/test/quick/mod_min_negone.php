<?hh

function int_min() :mixed{
  return ~PHP_INT_MAX;
}

function int_negone() :mixed{
  return -1;
}

<<__EntryPoint>> function main(): void {
  var_dump(int_min() % -1);
  var_dump(int_min() % int_negone());
}

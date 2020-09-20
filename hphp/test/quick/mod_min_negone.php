<?hh

function int_min() {
  return ~PHP_INT_MAX;
}

function int_negone() {
  return -1;
}

<<__EntryPoint>> function main(): void {
  var_dump(int_min() % -1);
  var_dump(int_min() % int_negone());
}

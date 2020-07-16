<?hh

const BLARF = darray[1 => "42", 51 => "area"];

function const_fold() {
  return \array_merge(BLARF);
}

<<__ProvenanceSkipFrame>>
function retag($x) {
  return darray(dict($x));
}

function wrap_retag() {
  return retag(const_fold());
}

<<__EntryPoint>>
function main () {
  $foo = const_fold();
  $bar = wrap_retag();
  $baz = darray(dict(const_fold()));
  var_dump(HH\get_provenance(__hhvm_intrinsics\launder_value($foo)));
  var_dump(HH\get_provenance(__hhvm_intrinsics\launder_value($bar)));
  var_dump(HH\get_provenance(__hhvm_intrinsics\launder_value($baz)));
}


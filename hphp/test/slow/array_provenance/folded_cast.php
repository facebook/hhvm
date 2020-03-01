<?hh

const BLARF = darray[1 => "42", 51 => "area"];

function blarf() {
  return \array_flip(BLARF);
}

<<__ProvenanceSkipFrame>>
function darray_shallow($x) {
  return darray($x);
}

function zorp() {
  return darray_shallow(blarf());
}

<<__EntryPoint>>
function main () {
  $foo = zorp();
  $bar = darray(blarf());
  var_dump(HH\get_provenance(__hhvm_intrinsics\launder_value($foo)));
  var_dump(HH\get_provenance(__hhvm_intrinsics\launder_value($bar)));
}


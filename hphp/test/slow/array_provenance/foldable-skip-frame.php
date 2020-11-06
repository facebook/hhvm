<?hh

<<__ProvenanceSkipFrame>>
function afk_psf($key, $n) {
  return array_fill_keys($key, $n);
}

function afk($key, $n) {
  return array_fill_keys($key, $n);
}

<<__EntryPoint>>
function main() {
  $x = afk_psf(varray[17], 34);
  print(HH\get_provenance($x)."\n");

  $x = afk(varray[17], 34);
  print(HH\get_provenance($x)."\n");
}

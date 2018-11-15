<?hh

<<__EntryPoint>>
function main() {
  var_dump(array_unique(keyset["1", 1, 2, "2"], SORT_STRING));
  var_dump(array_unique(keyset["1", 1, 2, "2"], SORT_NUMERIC));
  var_dump(array_unique(keyset["1", 1, 2, "2"], SORT_REGULAR));

}

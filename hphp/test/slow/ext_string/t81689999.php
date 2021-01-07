<?hh

<<__EntryPoint>>
function main() {
  var_dump(str_getcsv("\n\n", "", "\x00"));
}

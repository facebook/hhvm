<?hh

<<__EntryPoint>>
function main() :mixed{
  var_dump(str_getcsv("\n\n", "", "\x00"));
}

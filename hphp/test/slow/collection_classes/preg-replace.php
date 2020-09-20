<?hh
function main() {
  var_dump(preg_replace("/a/","b", HH\Set {"aa", "bb"}));
}

<<__EntryPoint>>
function main_preg_replace() {
main();
}

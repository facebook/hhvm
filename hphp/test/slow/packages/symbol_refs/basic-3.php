<?hh

module a;

<<__EntryPoint>>
function main_basic_3() {
  foo(); // in module b which not in same package as a
  echo "Done\n";
}

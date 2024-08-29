<?hh

module bar;

<<__EntryPoint>>
function main_basic_1() :mixed{
  foo(); // in module foo which is not in same package as bar
  echo "Done\n";
}

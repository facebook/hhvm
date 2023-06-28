<?hh

module a;

<<__EntryPoint>>
function main_basic_2() :mixed{
  foo(); // in module a which is in same package as a
  echo "Done\n";
}

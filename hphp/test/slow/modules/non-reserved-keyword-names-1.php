<?hh

new module file {}
new module foo.file {}
new module file.foo {}
new module default {}

<<__EntryPoint>>
function main() :mixed{
  echo "Done\n";
}

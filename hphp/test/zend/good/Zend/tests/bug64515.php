<?hh
function foo($unused = null, $unused = null, $arg = array()) {
  return 1;
}
<<__EntryPoint>> function main(): void {
foo();
echo "okey";
}

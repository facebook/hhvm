<?hh
function foo($unused = null, $unused = null, $arg = varray[]) :mixed{
  return 1;
}
<<__EntryPoint>> function main(): void {
foo();
echo "okey";
}

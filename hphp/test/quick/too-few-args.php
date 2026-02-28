<?hh
function foo($bar, $baz) :mixed{
  return 42;
}
<<__EntryPoint>> function main(): void {
var_dump(foo());
}

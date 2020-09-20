<?hh

function foo() {
  if (true) { return false; }
  else { return true; }
}
<<__EntryPoint>> function main(): void {
var_dump(foo());
}

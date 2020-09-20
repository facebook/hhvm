<?hh

function foo($x) {
  return 30000000000;
}
<<__EntryPoint>> function main(): void {
echo foo(34) . "\n";
}

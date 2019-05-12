<?hh

function foo($x) {
  return "OLD";
}
<<__EntryPoint>> function main(): void {
echo foo(34) . "\n";
}

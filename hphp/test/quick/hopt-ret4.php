<?hh

function foo($x) :mixed{
  return "OLD";
}
<<__EntryPoint>> function main(): void {
echo foo(34) . "\n";
}

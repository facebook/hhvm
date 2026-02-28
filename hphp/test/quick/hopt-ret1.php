<?hh

function foo($x) :mixed{
  return $x + 1;
}
<<__EntryPoint>> function main(): void {
echo foo(3) . "\n";
}

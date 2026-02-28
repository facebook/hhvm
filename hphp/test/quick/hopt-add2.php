<?hh

function foo($x, $y) :mixed{
  if ($x + $y) { return $x; }
  else { return 2; }
}
<<__EntryPoint>> function main(): void {
echo "foo(): " . foo(1, 2) . "\n";
}

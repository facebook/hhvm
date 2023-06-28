<?hh

function foo() :mixed{
  $x = 3;
  if (2 + $x) { return $x; }
  else { return 2; }
}
<<__EntryPoint>> function main(): void {
echo "foo(): " . foo() . "\n";
}

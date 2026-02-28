<?hh

function foo() :mixed{
  return 1 + 2;
}

function bar($x = 10) :mixed{
  return $x + (1 + 2);
}
<<__EntryPoint>> function main(): void {
echo bar() . "\n";
}

<?hh
function foo($a, $b) :mixed{
  // const - const
  $a = 5;
  $b = 7;
  return $a - $b;
}

<<__EntryPoint>> function main(): void {
  echo foo(5, 7);
  echo "\n";
}

<?hh
function foo($a, $b) :mixed{
  // const - reg
  $a = 5;
  return $a - $b;
}

<<__EntryPoint>> function main(): void {
  echo foo(5, 7);
  echo "\n";
}

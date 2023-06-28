<?hh
function foo($a, $b) :mixed{
  // reg - reg
  return $a - $b;
}

<<__EntryPoint>> function main(): void {
  echo foo(5, 7);
  echo "\n";
}

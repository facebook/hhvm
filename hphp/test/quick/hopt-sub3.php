<?hh
function foo($a, $b) :mixed{
  // reg - const
  $b = 7;
  return $a - $b;
}

<<__EntryPoint>> function main(): void {
  echo foo(5, 7);
  echo "\n";
}

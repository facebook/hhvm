<?hh

function foo($a) :mixed{
  list($x, $y) = 'x'.$a;
  return $x + $y;
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }

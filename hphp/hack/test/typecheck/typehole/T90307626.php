<?hh

function f(dynamic $d) : Map<string, int> {
  $output = Map<string, int> {};
  if ($d is string) {
    $output[$d] = 1;
  } else {
    $output[$d] = 2;
  }
  return $output;
}

function expect_string(string $s) : void {}

<<__EntryPoint>> function main() : void {
  // f(null); hhvm crashes inside f
  $m = f(1);
  $k = $m->keys()[0]; // k : string but contains an int
  expect_string($k);
}

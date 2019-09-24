<?hh

function bar(bool $k) {
  $x = null;
  for (;;) {
    $x = array('x' => $x);
    if ($k) return $x;
  }
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }

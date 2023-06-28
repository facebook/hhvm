<?hh

function bar(bool $k) :mixed{
  $x = null;
  for (;;) {
    $x = darray['x' => $x];
    if ($k) return $x;
  }
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }

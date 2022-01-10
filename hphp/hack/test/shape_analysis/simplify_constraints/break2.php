<?hh

function f(): void {
  $b = true;
  $d = dict[];
  while ($b) {
    $d['a'] = true;
    if ($b) { break; }
    $d['b'] = 42;
  }
  $d['c'] = 42.0;
}

<?hh

function f(): void {
  $b = true;
  $d = dict[];
  while ($b) {
    $d['a'] = true;
    continue;
    $d['b'] = 42;
  }
  $d['c'] = 42.0;
}

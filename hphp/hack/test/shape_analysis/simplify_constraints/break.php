<?hh

function f(): void {
  $b = true;
  $d = dict[];
  while ($b) {
    $d['a'] = true;
    break;
    $d['b'] = 42;
  }
  $d['c'] = 42.0;
}

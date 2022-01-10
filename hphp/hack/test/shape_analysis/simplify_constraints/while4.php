<?hh

function f(): void {
  $d = dict['a' => 42];
  $b = false;
  while ($b) {
    $d['c'] = false;
    $d = dict['b' => 'h'];
  }
}

<?hh

function f(): void {
  $d = dict['a' => 42];
  $b = false;
  while ($b) {
    $d['b'] = 'hi';
    inspect($d);
  }
  inspect($d);
}

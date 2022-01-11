<?hh

function f(): void {
  $d = dict['a' => 42];
  $c = true;
  if ($c) {
    $d['b'] = 'hi';
    inspect($d);
  } else {
    $d['c'] = true;
    inspect($d);
  }
  inspect($d);
}

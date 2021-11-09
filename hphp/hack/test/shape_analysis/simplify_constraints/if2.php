<?hh

function f(): void {
  $d = dict['a' => 42];
  $c = true;
  if ($c) {
    $d['b'] = 'hi';
  } else {
    $d['c'] = true;
  }
}

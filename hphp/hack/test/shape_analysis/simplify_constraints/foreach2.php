<?hh

function f(): void {
  $v = vec[dict['a' => 42]];
  foreach ($v as $d) {
    $d['b'] = true;
    inspect($d);
  }
  inspect($v[0]);
}

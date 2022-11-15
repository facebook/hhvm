<?hh

function f(): void {
  $v = vec[dict['a' => 42]];
  foreach ($v as $d) {
    $v[] = dict['b' => $d['a']];
    inspect($v[0]);
  }
  inspect($v[0]);
}

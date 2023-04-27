<?hh

function f(vec<bool> $v): void {
  $d = dict['d' => 42];
  $e = dict['e' => 'hi'];
  $f = dict['f' => 42.0];
  foreach ($v as $val) {
    $f['fixpoint'] = $val;
    inspect($f);
    $f = $e;
    $e = $d;
  }
}

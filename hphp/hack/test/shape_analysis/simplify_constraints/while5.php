<?hh

function f(): void {
  $b = false;
  $d = dict['d' => 42];
  $e = dict['e' => 'hi'];
  $f = dict['f' => 42.0];
  while ($b) {
    $f['fixpoint'] = true;
    inspect($f);
    $f = $e;
    $e = $d;
  }
}

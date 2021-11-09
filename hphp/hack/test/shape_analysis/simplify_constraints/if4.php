<?hh

function f(): void {
  $b = true;
  if ($b) {
    if ($b) {
      $d = dict['a' => 42];
    } else {
      $d = dict['b' => 42.0];
    }
  } else {
    if ($b) {
      $d = dict['c' => true];
    } else {
      if ($b) {
        $d = dict['d' => false];
      } else {
        $d = dict['e' => 42];
      }
    }
  }
  $d['f'] = 42;
}

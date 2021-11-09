<?hh

function f(): void {
  $d = dict[];
  $b = true;
  if ($b) {
    $d = dict[];
  }
  $d['a'] = 42;
}

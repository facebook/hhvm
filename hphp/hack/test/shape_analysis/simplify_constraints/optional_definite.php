<?hh

function f(): void {
  $d = dict[];
  $b = true;
  if ($b) {
    $d['a'] = 42;
  }
  inspect($d);
  $d['a'] = 24;
  inspect($d);
}

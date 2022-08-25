<?hh

function f(): void {
  $d = dict[];
  $b = true;
  if ($b) {
    $d['a'] = 'apple';
  }
  $d['a'] = 'apple';
  inspect($d);
}

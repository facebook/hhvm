<?hh

function f(): void {
  $d = dict['a' => true];
  $b = false;
  while ($b) {
    $d = dict['b' => 'h'];
  }
  inspect($d);
}

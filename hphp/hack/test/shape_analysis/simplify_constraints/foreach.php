<?hh

function f(): void {
  $d = dict['a' => 42];
  foreach ($d as $k => $v) {}
  inspect($d);
}

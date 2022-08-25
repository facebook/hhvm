<?hh

function f(): void {
  $d = dict['a' => 42];
  idx($d,'a');
  inspect($d);
}

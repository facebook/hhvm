<?hh

function f(): void {
  $x = dict['a' => 42];
  $x['b'];
  inspect($x);
}

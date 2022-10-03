<?hh

function f(): void {
  $d = tuple(dict['a' => 42], dict['b' => 'string']);
  inspect($d[0]);
  inspect($d[1]);
}

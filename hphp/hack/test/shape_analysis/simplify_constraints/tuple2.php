<?hh

function f(string $key): void {
  $d = tuple(dict['a' => 42], dict['b' => 'string']);
  $d[0][$key];
  inspect($d[0]);
  inspect($d[1]);
}

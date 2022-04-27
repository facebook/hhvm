<?hh

function f(bool $b): void {
  $d = $b ? dict['a' => 42] : dict['b' => 24];
  inspect($d);
}

<?hh

function f(dict<string, mixed> $d1, dict<string, mixed> $d2): void {
  $d1['k'] = 42;
  $d2['k'] = true;

  inspect($d1);
  inspect($d2);
}

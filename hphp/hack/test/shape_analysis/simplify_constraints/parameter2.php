<?hh

function f(int $i, dict<string, int> $d): void {
  $d['k'] = $i;
  inspect($d);
  $d['l'] = $i;
  inspect($d);
}

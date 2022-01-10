<?hh

function f(int $i, dict<string, int> $d): void {
  $d['k'] = $i;
  $d['l'] = $i;
}

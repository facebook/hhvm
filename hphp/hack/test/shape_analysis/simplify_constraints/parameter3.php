<?hh

function f(dict<string, int> $d): void {
  $d = dict[];
  $d['k'] = 42;
}

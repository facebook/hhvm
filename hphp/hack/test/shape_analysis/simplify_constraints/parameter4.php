<?hh

function f(dict<string, mixed> $d): void {
  $d['a'] = 42;
  $d['b'];
}

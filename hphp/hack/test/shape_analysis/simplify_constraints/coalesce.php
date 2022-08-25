<?hh

function f(dict<string,mixed> $d): void {
  $d['a'] ?? $d['b'];
  inspect($d);
}

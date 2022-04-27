<?hh

function f(dict<string, mixed> $d): void {
  $d['key1'] ?: $d['key2'];
}

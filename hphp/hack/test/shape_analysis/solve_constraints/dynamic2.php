<?hh

function f(dict<string, mixed> $d): void {
  $d['b'];
}

function main(): void {
  $x = 'a';
  f(dict[$x => 42]);
}

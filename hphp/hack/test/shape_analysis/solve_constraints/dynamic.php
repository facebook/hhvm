<?hh

function f(dict<string, mixed> $d) : void {
  $x = 'a';
  $d[$x];
}

function main(): void {
  f(dict['a' => 42]);
}

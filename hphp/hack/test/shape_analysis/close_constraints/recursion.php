<?hh

function f(dict<string, mixed> $d) : void {
  $d['a'];
  g($d);
}

function g(dict<string, mixed> $d) : void {
  $d['b'];
  f($d);
}

function main(): void {
  f(dict['a' => true, 'b' => 42]);
}

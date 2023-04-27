<?hh

function f(): void {
  dict['a' => 42, 'b' => true];
  dict['c' => 42];
}

function g(): void {
  dict['a' => 42, 'b' => true];
}

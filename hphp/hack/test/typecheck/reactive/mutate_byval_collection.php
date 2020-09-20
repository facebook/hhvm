<?hh // strict
<<__RxLocal>>
function test_function1(array<string, mixed> $arr): void {
  // OK
  $arr['herp'] = 7;
}

<<__RxLocal>>
function test_function2((int, int) $arr): void {
  // OK
  $arr[0] = 7;
}

<<__RxLocal>>
function test_function3(shape('x' => int) $arr): void {
  // OK
  $arr['x'] = 7;
}

<?hh

function foo(shape(?'bar' => int) $value): void {
  $value['bar'];
}

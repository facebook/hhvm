<?hh

function foo(shape('x' => int, ...) $s): void {
  //         ^ hover-at-caret
  throw new Exception();
}

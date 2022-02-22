<?hh

function foo(shape('x' => int) $arg): void {
  $x = $arg['x'];
  //        ^ hover-at-caret
}

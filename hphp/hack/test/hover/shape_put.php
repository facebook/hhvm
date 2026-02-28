<?hh

function foo(shape('x' => shape('y' => int)) $arg): void {
  $new_shape = Shapes::put($arg, 'x', shape('z' => 3));
  //    ^ hover-at-caret
}

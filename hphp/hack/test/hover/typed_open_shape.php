<?hh

function foo(shape('a' => int, string...) $arg): void {
  $x = $arg;
  //    ^ hover-at-caret
}

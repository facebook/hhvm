<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

function test_ellipsis_on_rhs(): void {
  $s = shape('x' => 1, ...);
  $t = tuple(1, 2, ...);
}

function test_question_on_rhs(): void {
  $s = shape(?'x' => 1);
  $t = shape(?$x);
}

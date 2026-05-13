<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Forbidden: destructuring in function parameter position
function test_param_shape(shape('x' => $x) $s): void {}

function test_param_tuple(tuple($a, $b) $t): void {}

// Forbidden: destructuring in catch binding
function test_catch_shape(): void {
  try {
    throw new Exception("test");
  } catch (Exception shape('x' => $x)) {}
}

function test_catch_tuple(): void {
  try {
    throw new Exception("test");
  } catch (Exception tuple($a, $b)) {}
}

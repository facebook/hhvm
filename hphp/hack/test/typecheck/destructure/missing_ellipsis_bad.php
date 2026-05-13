<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

function test_tuple_arity_mismatch((int, optional string) $t): void {
  tuple($x) = $t; // error: Tuple arity mismatch

}

function test_open_shape(shape('x' => int, ...) $s): void {
  shape('x' => $x) = $s; // error: Missing `...`
}

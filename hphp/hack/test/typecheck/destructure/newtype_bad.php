//// def.php
<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

newtype ShapeAlias as shape('x' => int, ...) = shape('x' => int, 'y' => string);

//// use.php
<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Cross-file: 'y' is hidden behind the newtype upper bound
function test_cross_file_hidden_field(ShapeAlias $s): void {
  shape('x' => $x, 'y' => $y, ...) = $s; // Field 'y' does not exist in the shape type
}

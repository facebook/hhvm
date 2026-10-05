<?hh
<<file:__EnableUnstableFeatures('shape_splat_expression')>>

function missing_comma(mixed $base): void {
  $_ = shape(...$base 'b' => 2);
}

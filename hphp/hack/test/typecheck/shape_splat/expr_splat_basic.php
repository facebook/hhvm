<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function test(): void {
  $defaults = shape('page_size' => 10, 'timeout' => 30);
  $overrides = shape('timeout' => 60, 'retries' => 3);
  $config = shape(...$defaults, ...$overrides);
  hh_expect<shape('page_size' => int, 'retries' => int, 'timeout' => int)>($config);
}

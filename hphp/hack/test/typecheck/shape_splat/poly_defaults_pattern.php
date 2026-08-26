<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// The defaults pattern with concrete type aliases
type TDefaults = shape('page_size' => int, 'timeout' => int);
type TOverrides = shape(?'timeout' => int, 'retries' => int);
type TConfig = shape(...TDefaults, ...TOverrides);

function test(TConfig $config): void {
  hh_expect<shape('page_size' => int, 'retries' => int, 'timeout' => int)>($config);
}

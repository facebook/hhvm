<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Enabling the type-level feature must not also enable expression splats.
function test(mixed $fields): void {
  $_ = shape(...$fields);
}

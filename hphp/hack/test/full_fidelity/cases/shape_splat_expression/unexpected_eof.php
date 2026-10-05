<?hh
<<file:__EnableUnstableFeatures('shape_splat_expression')>>

function unexpected_eof(mixed $fields): void {
  $_ = shape(...$fields,

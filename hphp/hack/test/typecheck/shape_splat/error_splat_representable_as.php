<?hh
<<file:__EnableUnstableFeatures(
  'representable_as',
  'shape_splat_concrete',
  'shape_splat_type_parameters',
)>>

// Representational compatibility does not make a type a shape. In particular,
// RepresentableAs<shape(...)> is deliberately one-way and cannot be unpacked.
function test(
  shape(
    ...\HH\Runtime\RepresentableAs<shape('x' => int)>,
    'y' => string,
  ) $_,
): void {}

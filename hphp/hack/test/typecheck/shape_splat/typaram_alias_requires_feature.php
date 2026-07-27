<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// A transparent type alias that splats its own type parameter is a
// type-parameter splat: it requires `shape_splat_type_parameters`, not just
// `shape_splat_concrete`. This should error. (Splatting a concrete alias or
// inline shape, by contrast, is fine under `shape_splat_concrete` alone.)
type WithTParam<T> = shape(...T, 'foo' => int);

type Concrete = shape('a' => int);
type Ok = shape(...Concrete, 'foo' => int);

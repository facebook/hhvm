//// definitions.php
<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

newtype Wrapper<T as shape(...)> as T = T;

//// usage.php
<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// SOUND ACCEPTANCE CONTROL: one use of the opaque newtype is expanded enough
// to prove that it cannot provide `b`.
<<__DisjointShapeSplat>>
type Single = shape(...Wrapper<shape('a' => int)>, 'b' => int);

// REGRESSION (previously a false positive): the inner and outer `Wrapper`
// instantiations are finite and must both be expanded.
<<__DisjointShapeSplat>>
type Double = shape(...Wrapper<Wrapper<shape('a' => int)>>, 'b' => int);

// TRUE POSITIVE CONTROL: repeated use of the constructor must not hide an
// overlap exposed by the innermost bound.
<<__DisjointShapeSplat>>
type BadDouble = shape(...Wrapper<Wrapper<shape('b' => int)>>, 'b' => bool);

<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// This complete dependency graph exposes exponential corner enumeration in
// `Typing_corners.corner_assignments`. Two parameters keep it within LSP's
// 60-second per-file limit; increase the count as that implementation improves.
<<__DisjointShapeSplat>>
newtype DenseCycle<
  T0 as shape(...T1),
  T1 as shape(...T0),
> = shape(...T0, 'x' => bool);

type Witness = DenseCycle<
  shape('x' => int),
  shape('x' => int),
>;

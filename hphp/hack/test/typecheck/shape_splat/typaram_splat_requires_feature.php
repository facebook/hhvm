<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Splatting a concrete shape alias is admitted by `shape_splat_concrete`, but
// splatting a *type parameter* additionally requires
// `shape_splat_type_parameters`. Only the latter is missing here, so the splat
// of `T` is an error while the splat of `C` is fine.

type C = shape('a' => int);

function ok(): shape(...C) {
  throw new Exception();
}

function bad<T as shape('a' => int)>(): shape(...T) {
  throw new Exception();
}

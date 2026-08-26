<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// `T` appears inside the splat operand as a type ARGUMENT, not as the operand
// itself, so it is not a spread type parameter: the splat well-formedness rules
// (shape bound, sole occurrence) do not apply to it, and `f` needs no bound on
// `T`. Compare splat_tparam_no_shape_bound.php, where `...T` is bare.
type Box<T> = shape('inner' => T);

function f<T>(shape(...Box<T>) $s): T {
  return $s['inner'];
}

function test(): void {
  hh_expect<int>(f(shape('inner' => 42)));
}

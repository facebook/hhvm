<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// A generic shape alias can be spread and its parameterised fields flow through.
type Box<T> = shape('inner' => T, 'tag' => string);

function unwrap<T>(shape('blah' => bool, ...Box<T>) $s): T {
  return $s['inner'];
}

function test(): void {
  $s = shape('blah' => false, 'inner' => 42, 'tag' => 'x');
  hh_expect<int>(unwrap($s));
}

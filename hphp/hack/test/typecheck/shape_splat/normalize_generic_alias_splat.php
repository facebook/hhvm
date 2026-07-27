<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Generic type alias that splats its own type parameter in the definition.
// (A transparent `type` alias cannot constrain its type param, so `T` is
// unbounded here; instantiating with a concrete shape folds via decl
// normalization. See normalize_generic_alias.php for the field-position variant.)
type TWith<T> = shape(...T, 'foo' => int);

function test(TWith<shape('a' => string)> $s): void {
  hh_expect<shape('a' => string, 'foo' => int)>($s);
  hh_expect<string>($s['a']);
  hh_expect<int>($s['foo']);
}

<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// A type-parameter bound with an ABSENT field: `?'name' => nothing` (an optional
// field of the uninhabited type) means "if `name` is present its value is
// impossible", i.e. `name` must be absent. So a shape with no `name` satisfies
// the bound and the call below is valid.
//
// The inference path has to reach the same conclusion as the ground/rigid one:
// the spread var's `name` field must read as Absent, not merely Optional, or it
// fails `Optional <: Absent` against the bound.
function needs_absent_name<T as shape(?'name' => nothing, ...)>(
  shape(...T) $s,
): void {}

function test(): void {
  $s = shape('id' => 42, 'age' => 30); // no 'name'
  needs_absent_name($s);
}

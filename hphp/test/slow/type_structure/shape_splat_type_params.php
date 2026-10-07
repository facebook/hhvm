<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Runtime resolution of shape-splat type structures whose splat operand is a
// TYPE PARAMETER. hackc cannot expand a type-parameter operand, and reflecting a
// generic alias by name leaves the parameter unbound, so `...T` stays an
// UNRESOLVED residual: the resolved type structure carries `splat_elem_types`
// (the merged concrete prefix as one leading shape element, then the
// type-parameter operand) rather than a flattened shape.

type TBase = shape('x' => int, 'y' => string);

// A type parameter after a concrete field: residual = [shape('a'=>int), T].
newtype TParam<T as shape(...)> = shape('a' => int, ...T);

// A concrete alias operand plus a trailing type parameter: the concrete prefix
// (shape('a'=>int) merged with TBase) collapses into a single leading element,
// followed by the unresolved type-parameter operand.
newtype TParamPrefix<T as shape(...)> = shape('a' => int, ...TBase, ...T);

// Known fields to the RIGHT of the type parameter: the parameter is leftmost, so
// the residual has an empty prefix and the concrete field trails it (it would win
// under rightmost-wins once T is known): residual = [T, shape('a'=>int)].
newtype TParamRight<T as shape(...)> = shape(...T, 'a' => int);

// Known fields on BOTH sides: a merged prefix, then the parameter, then the
// trailing concrete field: residual = [shape('a'=>int), T, shape('b'=>bool)].
newtype TParamBoth<T as shape(...)> = shape('a' => int, ...T, 'b' => bool);

// TWO shapes after the type parameter: everything trailing the parameter barrier
// must merge into a SINGLE element (canonical residual), not stay as separate
// splat elements: residual = [T, shape('x'=>int, 'y'=>string, 'z'=>bool)].
newtype TParamSuffix<T as shape(...)> = shape(...T, ...TBase, 'z' => bool);

<<__EntryPoint>>
function main(): void {
  var_dump(HH\type_structure_for_alias(nameof TParam));
  var_dump(HH\type_structure_for_alias(nameof TParamPrefix));
  var_dump(HH\type_structure_for_alias(nameof TParamRight));
  var_dump(HH\type_structure_for_alias(nameof TParamBoth));
  var_dump(HH\type_structure_for_alias(nameof TParamSuffix));
}

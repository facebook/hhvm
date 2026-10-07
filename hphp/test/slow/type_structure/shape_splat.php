<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'union_intersection_type_hints')>>

// Runtime resolution of shape-splat type structures. Exercises the merge in
// TypeStructure::mergeResolvedShapeSplat (and, under repo mode, the HHBBC
// mirror — both use the same merge, so the output must match across modes).

type TBase = shape('x' => int, 'y' => string);
type TRight = shape('x' => bool);
type TOpen = shape('z' => float, ...);

// disjoint merge + an extra literal field
type TExtended = shape(...TBase, 'z' => bool);
// rightmost-wins on 'x'
type TOverride = shape(...TBase, ...TRight);
// merging with an open shape yields an open result
type TOpenMerged = shape(...TBase, ...TOpen);
// splat of an inline literal shape
type TInline = shape(...TBase, ...shape('w' => int));
// dynamic on the left: concrete fields win; the row opens with dynamic as its
// unknown-field bound (variadic_type = dynamic)
type TDynLeft = shape(...dynamic, 'x' => int);
// dynamic on the right: the concrete field is unioned with dynamic
type TDynRight = shape('x' => int, ...dynamic);
// a union operand distributes outward:
//   shape(...(A|B)) = shape(...A) | shape(...B)
// so this resolves to a T_union of shape('x'=>int,'y'=>string) and shape('x'=>bool).
type TUnionSplat = shape(...(TBase | TRight));
// a trailing literal field is merged into each distributed branch (both branches
// gain 'z' => bool)
type TUnionTrailing = shape(...(TBase | TRight), 'z' => bool);
// rightmost-wins applies within each branch: 'x' becomes float in both members
type TUnionOverride = shape(...(TBase | TRight), 'x' => float);
// bare trailing `...` on an all-splat shape: the result is open and the splatted
// fields are widened to mixed by the open tail
type TBareOpen = shape(...TBase, ...);
// a field-run then a trailing `...`: the explicit 'z' keeps its type (bool),
// while the splatted TBase fields are widened to mixed. Must NOT widen 'z' — the
// openness attaches to the 'z' field-run, i.e. `...shape('z' => bool, ...)`.
type TFieldThenOpen = shape(...TBase, 'z' => bool, ...);
// typed open `int...`: unknown fields are bounded by int
type TTypedOpen = shape(...TBase, int...);

<<__EntryPoint>>
function main(): void {
  var_dump(HH\type_structure_for_alias(nameof TExtended));
  var_dump(HH\type_structure_for_alias(nameof TOverride));
  var_dump(HH\type_structure_for_alias(nameof TOpenMerged));
  var_dump(HH\type_structure_for_alias(nameof TInline));
  var_dump(HH\type_structure_for_alias(nameof TDynLeft));
  var_dump(HH\type_structure_for_alias(nameof TDynRight));
  var_dump(HH\type_structure_for_alias(nameof TUnionSplat));
  var_dump(HH\type_structure_for_alias(nameof TUnionTrailing));
  var_dump(HH\type_structure_for_alias(nameof TUnionOverride));
  var_dump(HH\type_structure_for_alias(nameof TBareOpen));
  var_dump(HH\type_structure_for_alias(nameof TFieldThenOpen));
  var_dump(HH\type_structure_for_alias(nameof TTypedOpen));
}

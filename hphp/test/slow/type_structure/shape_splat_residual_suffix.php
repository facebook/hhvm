<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

/* Reflecting a generic alias by name provides no concrete argument for `T`, so
   runtime resolution preserves `...T` as an unbound `T_typevar` residual. This
   residual is a merge barrier: concrete shape fragments cannot merge across it,
   but adjacent concrete fragments on either side must still merge. Consequently,
   the concrete suffix is normalized to one element, matching the typechecker's
   canonical residual representation. */

type TBase = shape('x' => int, 'y' => string);

/* Normalizes to:
   shape('x' => int, 'y' => string, 'z' => bool).
  `TBase` and the following `'z'` field form one concrete suffix */
newtype TSuffix<T as shape(...)> = shape(...T, ...TBase, 'z' => bool);

/* Normalizes to
   shape(...shape('a' => bool), T, ...shape('x' => int, 'y' => string, 'z' => bool))
*/
newtype TPrefixSuffix<T as shape(...)> =
  shape('a' => bool, ...T, ...TBase, 'z' => bool);

<<__EntryPoint>>
function main(): void {
  var_dump(HH\type_structure_for_alias(nameof TSuffix));
  var_dump(HH\type_structure_for_alias(nameof TPrefixSuffix));
  var_dump((new ReflectionTypeAlias(nameof TSuffix))->getAssignedTypeText());
}

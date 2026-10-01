<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'union_intersection_type_hints')>>

// REGRESSION (previously a false negative): `everything_sdt` wraps the open
// outer shape in `supportdyn`, but the overlapping `id` fields must be checked.
<<__DisjointShapeSplat>>
type Bad = shape(...shape('id' => int), 'id' => bool, ...);

// An explicit wrapper combines with the automatically inserted wrapper, so
// stripping `supportdyn` must be recursive.
<<__DisjointShapeSplat>>
type BadNestedSupportdyn =
  supportdyn<shape(...shape('nested_id' => int), 'nested_id' => bool, ...)>;

// A top-level union must recurse before peeling the implicit `supportdyn`
// wrapper from its open shape branch.
<<__DisjointShapeSplat>>
type BadUnion =
  (shape() |
    shape(...shape('union_id' => int), 'union_id' => bool, ...));

<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'union_intersection_type_hints')>>

type HasX = shape('x' => int);

// REGRESSION (previously a false negative): a top-level union must not hide an
// overlapping splat in its first branch.
<<__DisjointShapeSplat>>
type Bad =
  (shape(...HasX, 'x' => bool) | shape(...shape('a' => int), 'x' => int));

// Every union branch must be visited, not only the first one.
<<__DisjointShapeSplat>>
type BadSecond =
  (shape(...shape('a' => int), 'x' => int) | shape(...HasX, 'x' => bool));

// SOUND ACCEPTANCE CONTROL: fields in alternative branches are not compared
// with one another.
<<__DisjointShapeSplat>>
type Good =
  (shape(...shape(), 'x' => int, ?'a' => bool) |
    shape(...shape(), 'x' => int, ?'b' => bool));

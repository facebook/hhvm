<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'union_intersection_type_hints')>>

type HasX = shape('x' => int);

// REGRESSION (previously a false negative): a nullable wrapper must not hide
// an overlapping splat.
<<__DisjointShapeSplat>>
type BadOption = ?shape(...HasX, 'x' => bool);

// SOUND ACCEPTANCE CONTROL: the non-null branch is internally disjoint.
<<__DisjointShapeSplat>>
type GoodOption = ?shape(...shape(), 'x' => int);

// REGRESSION (previously a false negative): a top-level intersection must not
// hide an overlapping splat in one of its members.
<<__DisjointShapeSplat>>
type BadIntersection =
  (shape(...HasX, 'x' => bool) & shape('x' => bool));

// SOUND ACCEPTANCE CONTROL: fields in separate intersection members are not
// compared with one another.
<<__DisjointShapeSplat>>
type GoodIntersection =
  (shape(...shape(), 'x' => int, ?'a' => bool) &
    shape(...shape(), 'x' => int, ?'b' => bool));

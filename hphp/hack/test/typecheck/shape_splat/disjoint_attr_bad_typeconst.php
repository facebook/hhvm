<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

abstract class C {
  const type TFields = shape('a' => int);
}

// A splat of a concrete type constant expands to the shape it stands for, so
// this overlaps on 'a' just as a spread alias would.
<<__DisjointShapeSplat>>
type Bad = shape(...C::TFields, 'a' => string);

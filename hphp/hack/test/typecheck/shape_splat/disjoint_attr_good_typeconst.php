<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

abstract class C {
  const type TFields = shape('a' => int);
}

// C::TFields is closed and supplies only 'a', so 'b' cannot collide with it.
<<__DisjointShapeSplat>>
type Good = shape(...C::TFields, 'b' => string);

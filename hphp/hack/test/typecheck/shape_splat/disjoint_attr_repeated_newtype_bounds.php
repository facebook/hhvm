//// definitions.php
<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

newtype N6 as shape('a' => int) = nothing;
newtype N5 as shape(...N6, ...N6) = nothing;
newtype N4 as shape(...N5, ...N5) = nothing;
newtype N3 as shape(...N4, ...N4) = nothing;
newtype N2 as shape(...N3, ...N3) = nothing;
newtype N1 as shape(...N2, ...N2) = nothing;
newtype N0 as shape(...N1, ...N1) = nothing;

//// usage.php
<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

<<__DisjointShapeSplat>>
type Good = shape(...N0, 'b' => int);

<<__DisjointShapeSplat>>
newtype GoodThroughParameter<T as N0> = shape(...T, 'b' => int);

<<__DisjointShapeSplat>>
type Bad = shape(...N0, 'a' => string);

<<__DisjointShapeSplat>>
newtype BadThroughParameter<T as N0> = shape(...T, 'a' => string);

<<__DisjointShapeSplat>>
type RepeatedBad = shape(...N0, ...N0);

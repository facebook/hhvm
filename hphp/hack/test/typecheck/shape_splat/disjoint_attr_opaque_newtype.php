//// opaque_newtype_definition.php
<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

newtype OpaqueFields as shape('a' => int) = shape('a' => int);

//// opaque_newtype_use.php
<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

<<__DisjointShapeSplat>>
type BadDirect = shape(...OpaqueFields, 'a' => string);

<<__DisjointShapeSplat>>
newtype BadTransitive<T as OpaqueFields> = shape(...T, 'a' => string);

<<__DisjointShapeSplat>>
type GoodDirect = shape(...OpaqueFields, 'b' => string);

<<__DisjointShapeSplat>>
newtype GoodTransitive<T as OpaqueFields> = shape(...T, 'b' => string);

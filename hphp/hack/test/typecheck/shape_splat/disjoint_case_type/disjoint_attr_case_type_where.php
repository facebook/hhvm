<?hh
<<file:__EnableUnstableFeatures('case_types', 'case_type_where_clauses', 'shape_splat_concrete', 'shape_splat_type_parameters')>>

<<__DisjointShapeSplat>>
case type Good<T as shape(...)> =
  | shape(...T, 'x' => int) where T as shape(?'x' => nothing, ...)
  | int;

<<__DisjointShapeSplat>>
case type Bad<T as shape(...)> =
  | shape(...T, 'x' => int)
  | int;

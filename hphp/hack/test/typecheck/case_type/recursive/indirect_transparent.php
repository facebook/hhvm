//// file1.php
<?hh

<<file:__EnableUnstableFeatures('recursive_case_types')>>
// cycle [def A] : B -> A (opaque)
case type A = shape('a' => ?B);

//// file2.php
<?hh

// no cycle [def B] : A (opaque)
type B = shape('b' => A);

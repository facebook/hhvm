//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

new module A {}
new module B {}

//// A.php
<?hh

<<file:__EnableUnstableFeatures('modules')>>
module A;

internal type AInt = int;
internal type AVecInt = vec<AInt>; // Ok

//// B.php
<?hh

<<file:__EnableUnstableFeatures('modules')>>
module B;

internal type BVecInt = vec<AInt>; // Error
